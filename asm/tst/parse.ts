; Apache-2.0

.Main
.TestParseRegs
  ld hl, parseTable

.testParseLoop
  push hl
  ld c, (hl)
  inc hl
  ld b, (hl)
  xor a
  or b
  or c
  jr z, testParseEnd
  inc hl
  ld e, (hl)
  inc hl
  ld d, (hl)
  ld l, e
  ld h, d

  call TstParseRegs
  ld hl, =TstRegMask+2
  inc bc
  inc bc
  call cmpMask
  jr nz, fail
  pop hl
  inc hl
  inc hl
  inc hl
  inc hl
  ld a, (testnum)
  inc a
  ld (testnum), a
  jr testParseLoop

.testParseEnd
  pop hl
  ld bc, 0
  ret
.fail
  pop hl
  ld bc, (testnum)
  ret

; zero flag is clear on failure
.cmpMask
  ld e, 3
.cmpMLoop
  ld a, (bc)
  cpd
  ret nz
  dec e
  jr nz, cmpMLoop
  ret

.testnum
db 1,0
.res
db 0,0,0

.parseTable
dw test1Res
dw test1
dw test2Res
dw test2
dw test3Res
dw test3
dw test4Res
dw test4
dw test5Res
dw test5
dw test6Res
dw test6
dw test7Res
dw test7
dw 0

.test1Res
db $FF,$FF,$3
.test1
"abcdefhlixiya'f'b'c'd'e'h'l'"
db 0
.test2Res
db $96,$59,$2
.test2
"acehixa'c'e'h'"
db 0
.test3Res
db 0,0,0
.test3
db 0
.test4Res
db $0,$2B,$0
.test4
"ixiya'b'"
db 0
.test5Res
db 0,0,0
.test5
"abix'"
db 0
.test6Res
db 0,0,0
.test6
"abcdefg"
db 0
.test7Res
db $2,$8,$0
.test7
"aaa'a'"
db 0

.TestCmpRegsA
  call TstSaveRegs
  push bc
  push af
  cpl
  ld c, a
  pop af
  ld a, c
  pop bc
  call TstCmpRegs
  ld bc, =cmpResA+2
  jp testCmpR

.TestCmpRegsF
  xor a
  call TstSaveRegs
  scf
  call TstCmpRegs
  ld bc, =cmpResF+2
  jp testCmpR

.TestCmpRegsC
  call TstSaveRegs
  push af
  ld a, c
  cpl
  ld c, a
  pop af
  call TstCmpRegs
  ld bc, =cmpResC+2
  jp testCmpR

.TestCmpRegsB
  call TstSaveRegs
  push af
  ld a, b
  cpl
  ld b, a
  pop af
  call TstCmpRegs
  ld bc, =cmpResB+2
  jp testCmpR

.TestCmpRegsE
  call TstSaveRegs
  push af
  ld a, e
  cpl
  ld e, a
  pop af
  call TstCmpRegs
  ld bc, =cmpResE+2
  jp testCmpR

.TestCmpRegsD
  call TstSaveRegs
  push af
  ld a, d
  cpl
  ld d, a
  pop af
  call TstCmpRegs
  ld bc, =cmpResD+2
  jp testCmpR

.TestCmpRegsL
  call TstSaveRegs
  push af
  ld a, l
  cpl
  ld l, a
  pop af
  call TstCmpRegs
  ld bc, =cmpResL+2
  jp testCmpR

.TestCmpRegsH
  call TstSaveRegs
  push af
  ld a, h
  cpl
  ld h, a
  pop af
  call TstCmpRegs
  ld bc, =cmpResH+2
  jp testCmpR

.TestCmpRegsIX
  call TstSaveRegs
  push af
  push ix
  pop af
  cpl
  push af
  pop ix
  pop af
  call TstCmpRegs
  ld bc, =cmpResIX+2
  jp testCmpR

.TestCmpRegsIY
  call TstSaveRegs
  di
  push iy
  push af
  push iy
  pop af
  cpl
  push af
  pop iy
  pop af
  call TstCmpRegs
  pop iy
  ei
  ld bc, =cmpResIY+2
  jp testCmpR

.testCmpR
  ld hl, =TstChangedRegMask+2
  call cmpMask
  jr nz, cmpFail
  ld bc, 0
  ret
.cmpFail
  ld bc, 1
  ret

.TestCmpRegsAP
  di
  ex af, af'
  push af
  ex af, af'
  call TstSaveRegs
  ex af, af'
  push bc
  push af
  cpl
  ld c, a
  pop af
  ld a, c
  pop bc
  ex af, af'
  call TstCmpRegs
  ld bc, =cmpResAP+2
  ex af, af'
  pop af
  ex af, af'
  ei
  jp testCmpR

.TestCmpRegsFP
  di
  ex af, af'
  push af
  ex af, af'
  xor a
  call TstSaveRegs
  ex af, af'
  scf
  ex af, af'
  call TstCmpRegs
  ld bc, =cmpResFP+2
  ex af, af'
  pop af
  ex af, af'
  ei
  jp testCmpR

.TestCmpRegsCP
  di
  exx
  push bc
  exx
  call TstSaveRegs
  push af
  exx
  ld a, c
  cpl
  ld c, a
  exx
  pop af
  call TstCmpRegs
  ld bc, =cmpResCP+2
  exx
  pop bc
  exx
  ei
  jp testCmpR

.TestCmpRegsBP
  di
  exx
  push bc
  exx
  call TstSaveRegs
  push af
  exx
  ld a, b
  cpl
  ld b, a
  exx
  pop af
  call TstCmpRegs
  ld bc, =cmpResBP+2
  exx
  pop bc
  exx
  ei
  jp testCmpR

.TestCmpRegsEP
  di
  exx
  push de
  exx
  call TstSaveRegs
  push af
  exx
  ld a, e
  cpl
  ld e, a
  exx
  pop af
  call TstCmpRegs
  ld bc, =cmpResEP+2
  exx
  pop de
  exx
  ei
  jp testCmpR

.TestCmpRegsDP
  di
  exx
  push de
  exx
  call TstSaveRegs
  push af
  exx
  ld a, d
  cpl
  ld d, a
  pop af
  exx
  call TstCmpRegs
  ld bc, =cmpResDP+2
  exx
  pop de
  exx
  ei
  jp testCmpR

.TestCmpRegsLP
  di
  exx
  push hl
  exx
  call TstSaveRegs
  push af
  exx
  ld a, l
  cpl
  ld l, a
  exx
  pop af
  call TstCmpRegs
  ld bc, =cmpResLP+2
  exx
  pop hl
  exx
  ei
  jp testCmpR

.TestCmpRegsHP
  di
  exx
  push hl
  exx
  call TstSaveRegs
  push af
  exx
  ld a, h
  cpl
  ld h, a
  exx
  pop af
  call TstCmpRegs
  ld bc, =cmpResHP+2
  exx
  pop hl
  exx
  ei
  jp testCmpR

.TestCmpRegsMulti
  di
  exx
  push hl
  exx
  xor a
  call TstSaveRegs
  push af
  ld a, c
  cpl
  ld c, a
  ld a, b
  cpl
  ld b, a
  exx
  ld a, c
  cpl
  ld c, a
  ld a, b
  cpl
  ld b, a
  ld a, l
  cpl
  ld l, a
  ld a, h
  cpl
  ld h, a
  exx
  pop af
  cpl
  scf
  call TstCmpRegs
  ld bc, =cmpResMulti+2
  exx
  pop hl
  exx
  ei
  jp testCmpR

.cmpResA
db $2,$0,$0
.cmpResF
db $1,$0,$0
.cmpResC
db $4,$0,$0
.cmpResB
db $8,$0,$0
.cmpResE
db $10,$0,$0
.cmpResD
db $20,$0,$0
.cmpResL
db $40,$0,$0
.cmpResH
db $80,$0,$0
.cmpResIX
db $0,$1,$0
.cmpResIY
db $0,$2,$0
.cmpResAP
db $0,$8,$0
.cmpResFP
db $0,$4,$0
.cmpResCP
db $0,$10,$0
.cmpResBP
db $0,$20,$0
.cmpResEP
db $0,$40,$0
.cmpResDP
db $0,$80,$0
.cmpResLP
db $0,$0,$1
.cmpResHP
db $0,$0,$2
.cmpResMulti
db $F,$30,$3

