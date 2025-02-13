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
EXTERN _error_msgs

defc SPECASM_SAMAC_ORG_ADDRESS = 28736
defc SPECASM_SAMAC_TABLE = SPECASM_SAMAC_ORG_ADDRESS - 64
defc SPECASM_SAMAC_BUF_ADDRESS = SPECASM_SAMAC_TABLE + 24

_specasm_samac_init:
	ld bc, asmH
	ld (SPECASM_SAMAC_TABLE), bc

	ld bc, loadH
	ld (SPECASM_SAMAC_TABLE + 2), bc

	ld bc, newH
	ld (SPECASM_SAMAC_TABLE + 4), bc

	ld bc, saveH
	ld (SPECASM_SAMAC_TABLE + 6), bc

	; 56 bytes left for the buffer.  Let's set them to 0.
	ld hl, SPECASM_SAMAC_TABLE + 8
	ld de, SPECASM_SAMAC_TABLE + 9
	xor a
	ld (hl), a
	ld bc, 55
	ldir

	ret

; in:
;   a contains error number
; out:
;  bc contain pointer to error string

find_error:
	or a
	jr nz, got_an_error
	ld bc, 0
	ret

got_an_error:
	sla a
	ld h, 0
	ld l, a
	ld bc, _error_msgs
	add hl, bc
	ld c, (hl)
	inc hl
	ld b, (hl)
	ret

find_len:
	ld hl, SPECASM_SAMAC_BUF_ADDRESS
	xor a
	ld b, 40
find_len_loop:
	cp (hl)
	jr z, find_len_end
	inc hl
	djnz find_len_loop
find_len_end:
	ld a, 40
	sub b
	ld c, a
	ld b, 0
	ret

asmH:
	; note as we're calling into C code here we
	; need to duplicate what the z88dk crt does and
	; preserve certain registers and disable interrupts.
	; Otherwise it all gets a bit crashy.

	di
	push iy
	exx
	push hl
	exx

	call find_len
	push bc
	push SPECASM_SAMAC_BUF_ADDRESS
	call _specasm_samac_asm
	ld a, l
	pop de
	pop bc

	exx
	pop hl
	exx
	pop iy
	ei

	call find_error
	ret

newH:

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

	ld bc, 0

	ret

loadH:
	di
	push iy
	exx
	push hl
	exx

	call find_len
	push bc
	push SPECASM_SAMAC_BUF_ADDRESS
	call _specasm_samac_load
	ld a, l
	pop de
	pop bc

	exx
	pop hl
	exx
	pop iy
	ei

	call find_error
	ret

saveH:
	di
	push iy
	exx
	push hl
	exx

	call find_len
	push bc
	push SPECASM_SAMAC_BUF_ADDRESS
	call _specasm_samac_save
	ld a, l
	pop de
	pop bc

	exx
	pop hl
	exx
	pop iy
	ei

	call find_error
	ret
