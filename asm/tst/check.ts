; Apache-2.0

.TestChkBase
  ld hl, baseGood
  call TstParseRegs
  call TstSaveRegs
  call modAllBase
  call TstCheckRegs
  ret

.TestChkBaseBad
  ld hl, baseBad
  call TstParseRegs
  call TstSaveRegs
  call modAllBase
  call TstCheckRegs
  ld a, c
  or b
  jr z, badFail
  ld hl, baseGood
  call strcmp
  ret
.badFail
  ld bc, 1
  ret

.TestChkIXIY
  ld hl, ixiyGood
  call TstParseRegs
  call TstSaveRegs
  di
  push iy
  call modIXIY
  call TstCheckRegs
  pop iy
  ei
  ret

.TestChkIXIYBad
  ld hl, baseBad
  call TstParseRegs
  call TstSaveRegs
  di
  push iy
  call modIXIY
  call TstCheckRegs
  pop iy
  ei
  ld a, c
  or b
  jr z, badFail
  ld hl, ixiyGood
  call strcmp
  ret

.TestChkPrime
  ld hl, primeGood
  call TstParseRegs
  call TstSaveRegs
  call modAllPrime
  call TstCheckRegs
  ret

.TestChkPrimeBad
  ld hl, baseBad
  call TstParseRegs
  call TstSaveRegs
  call modAllPrime
  call TstCheckRegs
  ld a, c
  or b
  jr z, badFail
  ld hl, primeGood
  call strcmp
  ret

.TestChkMixed
  ld hl, mixedGood
  call TstParseRegs
  call TstSaveRegs
  call modMixed
  call TstCheckRegs
  ret

.TestChkMixedBad
  ld hl, baseBad
  call TstParseRegs
  call TstSaveRegs
  call modMixed
  call TstCheckRegs
  ld a, c
  or b
  jp z, badFail
  ld hl, mixedGood
  call strcmp
  ret

.strcmp
  ld a, (bc)
  cp (hl)
  jr nz, cmpFail
  or a
  jr z, cmpOk
  inc hl
  inc bc
  jr strcmp
.cmpFail
  ld bc, 1
  ret
.cmpOk
  ld bc, 0
  ret

.modAllBase
  push af
  pop bc
  ld a, b
  cpl
  ld b, a
  ld a, c
  cpl
  ld c, a
  push bc
  ld a, b
  cpl
  ld b, a
  ld a, c
  cpl
  ld c, a
  ld a, d
  cpl
  ld d, a
  ld a, e
  cpl
  ld e, a
  ld a, h
  cpl
  ld h, a
  ld a, l
  cpl
  ld l, a
  pop af
  ret

.modIXIY
  push af
  push ix
  pop af
  cpl
  push af
  pop ix
  push iy
  pop af
  cpl
  push af
  pop iy
  pop af
  ret

.modAllPrime
  exx
  ex af, af'
  call modAllBase
  ex af, af'
  exx
  ret

.modMixedBase
  push af
  ld a, b
  cpl
  ld b, a
  ld a, d
  cpl
  ld d, a
  ld a, h
  cpl
  ld h, a
  pop af
  cpl
  ret

.modMixed
  call modMixedBase
  ex af, af'
  exx
  call modMixedBase
  exx
  ex af, af'
  push ix
  pop af
  cpl
  push af
  pop ix
  ret


.baseGood
"facbedlh"
.baseBad
db 0
.ixiyGood
"ixiy"
db 0
.primeGood
"f'a'c'b'e'd'l'h'"
db 0
.mixedGood
"fabdhixf'a'b'd'h'"
db 0

