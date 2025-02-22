; Copyright contributors to Specasm
;
; Licensed under the Apache License, Version 2.0 (the "License");
; you may not use this file except in compliance with the License.
; You may obtain a copy of the License at
;
;      http://www.apache.org/licenses/LICENSE-2.0
;
; Unless required by applicable law or agreed to in writing, software
; distributed under the License is distributed on an "AS IS" BASIS,
; WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; See the License for the specific language governing permissions and
; limitations under the License.

SECTION code_compiler

PUBLIC _specasm_samac_init
EXTERN _specasm_samac_new
EXTERN _specasm_samac_asm
EXTERN _specasm_samac_load
EXTERN _specasm_samac_save

; Some SPECASM errors map to BASIC errors.  For everything else there's
; "Invalid Argument"

defc SPECASM_ERROR_TOO_MANY_LINES = 2
defc SPECASM_ERROR_NUM_TOO_BIG = 21
defc SPECASM_ERROR_BAD_FNAME = 23

; Addresses of ROM routines and System variables we need to subvert the error
; handling code.

defc CLEARSP=$1097
defc CLSLWR=$d6e
defc COPYBUF=$ecd
defc EXPTSTG=$1c8c
defc INCCHAD=$0074
defc OUTCODE=$15ef
defc OUTNUM=$1a1b
defc POMSGE=$c0a
defc SETMIN=$16b0
defc STKFTCH=$2bf1
defc STMRET=$1b76

defc CHADD=23645
defc DEFAD=23563
defc ERRNR=23610
defc ERRSP=23613
defc PPC=23621
defc STRM6=23574
defc XPTR=23647

_specasm_samac_init:
	ld hl, (ERRSP)
	ld de, extend
	ld (hl), e
	inc hl
	ld (hl), d
	ret

synend:
	pop hl
	bit 7, (iy+1)
	jr z, exit
	jp (hl)

exit:
	ld (iy+0), 255
	ld hl, extend
	push hl
	jp STMRET

error:
	bit 7, (iy+1)
	jr nz, runer

syner:
	ld hl, extend
	push hl
	ld hl, (CHADD)
	ld (XPTR), hl
	jp 4791

runer:
	res 5, (iy+1)
	bit 1, (iy+48)
	call nz, COPYBUF
	ld a, (ERRNR)
	inc a
	push af
	ld hl, 0
	ld (DEFAD), hl
	ld (iy+38), h
	ld (iy+55), h
	inc hl
	ld (STRM6), hl
	call SETMIN
	call CLSLWR
	set 5, (iy+2)
	pop af
	ld b, a
	cp 10
	jr c, numcod
	add a, 7

numcod:
	call OUTCODE
	ld a, 32
	rst 16
	ld a, b
	ld de, 5009
	call POMSGE
	xor a
	ld de, 5430
	call POMSGE
	ld bc, (PPC)
	call OUTNUM
	ld a, 58
	rst 16
	ld b, 0
	ld c, (iy+13)
	call OUTNUM
	call CLEARSP
	ld a, (ERRNR)
	ld (iy+0), 255
	ld hl, extend
	push hl
	jp 4968

fnameErr:
	ld a, $e
	jr exerr
numErr:
	ld a, $a
	jr exerr
noRoomErr:
	ld a, $f
	jr exerr
argErr:
	ld a, 9
exerr:
	ld (ERRNR), a
	jp error

; Entry point to our added Specasm commands.
extend:
; error needs to be Nonsense in
; Basic and next char needs to
; be a '*'
	ld a, (ERRNR)
	cp 11
	jp nz, error
	ld hl, (CHADD)
	ld a, (hl)
	cp '*'
	jp nz, error
	rst 24
	jp parse

; Each command is 8 bytes
; length 1 byte
; name 3 to 4 bytes
; args 1 byte (0 or 1)
; handler address
; optional padding byte

commands:
db 3, "asm"
db 1
dw asmH
db 0
db 3, "new"
db 0
dw newH
db 0
db 4, "load"
db 1
dw loadH
db 4, "save"
db 1
dw saveH
comEnd:

defc comCnt=(comEnd-commands)>>3

; returns if no match
parse:
	ld de, (CHADD)
	ld (chaddSv), de
	ld hl, commands
	ld (comSave), hl
	ld c, comCnt
findCom:
	ld b, (hl)
	inc hl
parseLoop:
	push hl
	rst 32
	pop hl
	cp (hl)
	jr z, checkNext
	dec c
	jr z, parseBad
	ld de, (chaddSv)
	ld (CHADD), de
	ld hl, (comSave)
	ld de, 8
	add hl, de
	ld (comSave), hl
	jr findCom
checkNext:
	inc hl
	djnz parseLoop
	push hl
	call INCCHAD
	pop hl
	ld b, a
	xor a
	cp (hl)
	jr nz, parseEnd
	ld a, b
	cp 13
	jr z, parseEnd
	cp ':'
	jr z, parseEnd
	jr parseBad
parseEnd:
	ld a, b
	inc hl
	ld e, (hl)
	inc hl
	ld d, (hl)
	ex de, hl
	; a has 1st ch of parameter
	jp (hl)
parseBad:
	ld de, (chaddSv)
	ld (CHADD), de
	jp error
comSave:
dw 0
chaddSv:
dw 0

samac_error:
	or a
	ret z
	pop hl
	cp SPECASM_ERROR_BAD_FNAME
	jp z, fnameErr
	cp SPECASM_ERROR_NUM_TOO_BIG
	jp z, numErr
	cp SPECASM_ERROR_TOO_MANY_LINES
	jp z, noRoomErr
	jp argErr

asmH:
	call EXPTSTG
	bit 6, (iy+1)
	jp nz, argErr
	call synend
	call STKFTCH

	; note as we're calling into C code here we
	; need to duplicate what the z88dk crt does and
	; preserve certain registers and disable interrupts.
	; Otherwise it all gets a bit crashy.

	di
	push iy
	exx
	push hl
	exx

	push bc
	push de
	call _specasm_samac_asm
	ld a, l
	pop de
	pop bc

	exx
	pop hl
	exx
	pop iy
	ei

	call samac_error
	jp exit

newH:
	call synend

	di
	push iy
	exx
	push hl
	exx

	call _specasm_samac_new

	exx
	pop hl
	exx
	pop iy
	ei

	jp exit

loadH:
	call EXPTSTG
	bit 6, (iy+1)
	jp nz, argErr
	call synend
	call STKFTCH

	di
	push iy
	exx
	push hl
	exx

	push bc
	push de
	call _specasm_samac_load
	ld a, l
	pop de
	pop bc

	exx
	pop hl
	exx
	pop iy
	ei

	call samac_error
	jp exit

saveH:
	call EXPTSTG
	bit 6, (iy+1)
	jp nz, argErr
	call synend
	call STKFTCH

	di
	push iy
	exx
	push hl
	exx

	push bc
	push de
	call _specasm_samac_save
	ld a, l
	pop de
	pop bc

	exx
	pop hl
	exx
	pop iy
	ei

	call samac_error
	jp exit
