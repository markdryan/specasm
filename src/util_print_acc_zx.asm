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

PUBLIC _specasm_util_print
PUBLIC _specasm_text_printch
PUBLIC _specasm_util_clear

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

	ld h, 0
	ld a, (bc)
	sla a
	rl h
	sla a
	rl h
	sla a
	rl h
	ld l, a
	ld a, h
	add a, $3C
	ld h, a

	; copy character
	push de

	; inner loop.  We save 22 tstate
	; s by unrolling

	ld a, (hl)
	inc hl
	ld (de), a
	inc d

	ld a, (hl)
	inc hl
	ld (de), a
	inc d

	ld a, (hl)
	inc hl
	ld (de), a
	inc d

	ld a, (hl)
	inc hl
	ld (de), a
	inc d

	ld a, (hl)
	inc hl
	ld (de), a
	inc d

	ld a, (hl)
	inc hl
	ld (de), a
	inc d

	ld a, (hl)
	inc hl
	ld (de), a
	inc d

	ld a, (hl)
	ld (de), a

	pop de

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
	jr z, clear_outer_end

clear_outer:
	ld (hl), c
	xor a

	push de

	ld (de), a
	inc d

	ld (de), a
	inc d

	ld (de), a
	inc d

	ld (de), a
	inc d

	ld (de), a
	inc d

	ld (de), a
	inc d

	ld (de), a
	inc d

	ld (de), a

	pop de

	inc de
	inc hl
	djnz clear_outer

clear_outer_end:

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

	ld h, 0
	ld a, b
	sla a
	rl h
	sla a
	rl h
	sla a
	rl h
	ld l, a
	ld a, h
	add a, $3C
	ld h, a

	ld a, (hl)
	inc hl
	ld (de), a
	inc d

	ld a, (hl)
	inc hl
	ld (de), a
	inc d

	ld a, (hl)
	inc hl
	ld (de), a
	inc d

	ld a, (hl)
	inc hl
	ld (de), a
	inc d

	ld a, (hl)
	inc hl
	ld (de), a
	inc d

	ld a, (hl)
	inc hl
	ld (de), a
	inc d

	ld a, (hl)
	inc hl
	ld (de), a
	inc d

	ld a, (hl)
	ld (de), a

	ret
