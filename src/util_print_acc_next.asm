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

defc SPECASM_NEXT_NUM_CHARS = 112
defc SPECASM_FONT_BUF_SIZE = SPECASM_NEXT_NUM_CHARS * 8

SECTION bss_compiler
PUBLIC _specasm_font
_specasm_font:
REPT SPECASM_FONT_BUF_SIZE
	db 0
ENDR

SECTION code_compiler

PUBLIC _specasm_util_print
PUBLIC _specasm_util_clear
PUBLIC _specasm_text_printch
PUBLIC _specasm_peer_next_copy_chars

prv_write_chars:
	ld hl, 0x5800

	ld a, 22
	rst 16
	ld a, 0
	rst 16
	ld a, 0
	rst 16
	ld a, 32
	ld b, SPECASM_NEXT_NUM_CHARS
write_char_loop:
	push af
	push hl
	push bc
	rst 16
	pop bc
	pop hl
	xor a
	ld (hl), a
	inc l
	pop af
	inc a
	djnz write_char_loop

	ret


_specasm_peer_next_copy_chars:
	push af
	push bc
	push de
	push hl

	call prv_write_chars
	ld hl, $4000
	ld de, _specasm_font

	ld b, SPECASM_NEXT_NUM_CHARS
copy_char_loop:
	push hl

	ld c, 8

copy_char_inner:
	ld a, (hl)
	ld (de), a
	inc de
	inc h

	dec c
	jr nz, copy_char_inner

	pop hl
	inc hl
	djnz copy_char_loop

	pop hl
	pop de
	pop bc
	pop af
	ret

; sp = ret
; sp + 2 = buf
; sp + 4 = x
; sp + 5 = y
; sp + 6 = attr
; specasm_util_print

; This should be 13011 tstates

_specasm_util_print:
	ld hl, 6
	add hl, sp
	ld a, (hl)
	ld (attr), a
	dec hl
	ld d, (hl)
	dec hl
	ld e, (hl)
	dec hl
	ld b, (hl)
	dec hl
	ld c, (hl)
	ex de, hl

	xor a
	ld (counter), a

	ld a, h
	ld d, h
	rrca
	rrca
	rrca
	ld h, a
	and $E0
	or l
	ld l, a
	ld e, a
	ld a, h
	and $3
	or $58
	ld h, a
	ld a, d
	and $18
	or $40
	ld d, a

	; hl attributes
	; de bitmap
	; bc string

outer:
	ld a, (bc)
	or a
	jr z, outerEnd
	ld a, (attr)
	ld (hl), a
	push hl

	; compute rom address

	push de

	ld hl, _specasm_font
	ld a, (bc)
	sub a, 32
	ld d, 8
	ld e, a
	mul d, e
	add hl, de

	pop de

	; copy character

	push bc
	push de

	ld b, 8
print_inner:
	ld a, (hl)
	inc hl
	ld (de), a
	inc d
	djnz print_inner

	pop de
	pop bc

	inc de
	inc bc
	ld hl, counter
	inc (hl)
	pop hl
	inc hl

	jr outer

outerEnd:
	ld a, (counter)
	ld l, a

	ret

attr:
	defb 0
counter:
	defb 0

; sp = ret
; sp + 2 = x
; sp + 3 = y
; sp + 4 = len
; sp + 5 = attr

_specasm_util_clear:
	ld hl, 5
	add hl, sp
	ld c, (hl)
	dec hl
	ld b, (hl)
	dec hl
	ld d, (hl)
	dec hl
	ld e, (hl)
	ex de, hl

	ld a, h
	ld d, h
	rrca
	rrca
	rrca
	ld h, a
	and $E0
	or l
	ld l, a
	ld e, a
	ld a, h
	and $3
	or $58
	ld h, a
	ld a, d
	and $18
	or $40
	ld d, a

	; c attr
	; hl attribute
	; de bitmap
	; b len

	ld a, b
	or a
	ret z

clear_outer:
	ld (hl), c
	xor a

	push de

	push bc
	ld b, 8
clear_inner:
	ld (de), a
	inc d
	djnz clear_inner
	pop bc

	pop de

	inc de
	inc hl
	djnz clear_outer

	ret

; sp = ret
; sp + 2 = ch
; sp + 3 = x
; sp + 4 = y
; sp + 5 = attr

_specasm_text_printch:
	ld hl, 5
	add hl, sp
	ld c, (hl)
	dec hl
	ld d, (hl)
	dec hl
	ld e, (hl)
	dec hl
	ld b, (hl)
	ex de, hl

	ld a, h
	ld d, h
	rrca
	rrca
	rrca
	ld h, a
	and $E0
	or l
	ld l, a
	ld e, a
	ld a, h
	and $3
	or $58
	ld h, a
	ld a, d
	and $18
	or $40
	ld d, a

	; hl attributes
	; de bitmap
	; b ch
	; c attr

	ld (hl), c

	; compute rom address

	; hl contains font address
	ld hl, _specasm_font
	push de
	ld a, b
	sub a, 32
	ld d, 8
	ld e, a
	mul d, e
	add hl, de
	pop de

	ld b, 8

printch_loop:
	ld a, (hl)
	inc hl
	ld (de), a
	inc d
	djnz printch_loop

	ret
