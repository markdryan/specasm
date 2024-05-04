; Apache-2.0

.saveRegs
  exx
  push hl
  push de
  push bc
  exx
  ex af, af'
  push af
  ex af, af'
  push iy
  push ix
  push hl
  push de
  push bc
  push af
  ld sp, (stackSp)
  ret

; Saves current registers to
; memory.
.TstSaveRegs
  ld (stackSp), sp
  ld sp, regEnd
  jr saveRegs

.saveCurRegs
  ld (stackSp), sp
  ld sp, curRegEnd
  jr saveRegs

.stackSp
dw 0

.zeroMask
  xor a
  ld (de), a
  inc de
  ld (de), a
  inc de
  ld (de), a
  dec de
  dec de
  ret

; Creates a 3 byte bitmap from
; a register list.  Registers
; are specified by name with
; no intervening spaces, e.g.
; afbcixa'f'h'

; hl reg string to parse
; 3 byte output mask stored
; in RegMask

; bits
; byte 0:f,a,c,b,e,d,l,h
; byte 1:ix,iy,f',a',c',b',e',d'
; byte 2:l',h'

.TstParseRegs
  push af
  push bc
  push de
  push hl
  push ix
  ld de, TstRegMask
  call zeroMask
.nextReg
  ld a, (hl)
  or a
  jr z, quit
  cp 'a'
  jr c, fail
  cp 'g'
  jr z, fail
  cp 'i'
  jr nc, checkLIxIy

; we have a,b,c,d,e,f,h,l
  sub 'a'
  ld ix, regLookup
  ld (=smc+2), a
.smc
  ld c, (ix+0)
  inc hl
  ld a, (hl)
  cp 39             ; '
  ld a, c
  jr nz, setBit
  add a, 10

.nextIter
  inc hl

.setBit
  push de
.setBitLoop
  cp 8
  jr c, endSetBitLoop
  sub 8
  inc de
  jr setBitLoop
.endSetBitLoop
  ld b, a
  inc b
  ld a, (de)
  ld c, a
  ld a, 1
.shiftLoop
  dec b
  jr z, shiftLoopEnd
  sla a
  jr shiftLoop
.shiftLoopEnd
  or c
  ld (de), a
  pop de
  jr nextReg
.fail
  call zeroMask

.quit
  pop ix
  pop hl
  pop de
  pop bc
  pop af
  ret

.checkLIxIy
  cp 'l'
  jr nz, notl
  inc hl
  ld a, (hl)
  cp 39             ; '
  jr nz, notlprime
  ld a, 16
  jr nextIter
.notlprime
  ld a, 6
  jr setBit
.notl
  cp 'i'
  jr nz, fail
  inc hl
  ld a, (hl)
  cp 'x'
  jr z, gotIx
  cp 'y'
  jr nz, fail
  ld a, 9
  jr nextIter
.gotIx
  ld a, 8
  jr nextIter

.regLookup
db 1                ; a
db 3                ; b
db 2                ; c
db 5                ; d
db 4                ; e
db 0                ; f
db 0
db 7                ; h
db 6                ; l

.TstRegMask
db 0,0,0

.TstChangedRegMask
.changed1
db 0
.changed2
db 0
.changed3
db 0

; Compares current registers
; to saved register set and
; checks that any modified
; registers are in the RegMask
; bc=0 on success
; bc points to error string
; on failure.
.TstCheckRegs
  call TstCmpRegs
  push af
  push hl
  push de
  push ix

  ld de, errorMsg
  ld b, 0
  ld a, (TstRegMask)
  cpl
  ld hl, changed1
  and (hl)
  ld c, a
  ld ix, regNames
.bRegLoop
  rr c
  jr nc, bRegIt
  ld a, b
  ld (=bRegSmc+2), a
.bRegSmc
  ld a, (ix+0)
  ld (de), a
  inc de
.bRegIt
  inc b
  ld a, b
  cp 8
  jr nz, bRegLoop

; process second block

  ld b, 0
  ld a, (=TstRegMask+1)
  cpl
  ld hl, =changed1+1
  and (hl)
  ld c, a
  rr c
  jr nc, notIX
  ld a, 'i'
  ld (de), a
  inc de
  ld a, 'x'
  ld (de), a
  inc de
.notIX
  rr c
  jr nc, notIY
  ld a, 'i'
  ld (de), a
  inc de
  ld a, 'y'
  ld (de), a
  inc de
.notIY

.bRegLoop2
  call primeBody
  cp 6
  jr nz, bRegLoop2

; process third block

  ld ix, =regNames+6
  ld b, 0
  ld a, (=TstRegMask+2)
  cpl
  ld hl, =changed1+2
  and (hl)
  ld c, a

.bRegLoop3
  call primeBody
  cp 2
  jr nz, bRegLoop3

  ld bc, errorMsg
  ld a, c
  cp e
  jr nz, checkError
  ld bc, 0
  jr checkEnd
.checkError
  ld bc, errorPre
  xor a
  ld (de), a

.checkEnd

  pop ix
  pop de
  pop hl
  pop af
  ret

.primeBody
  rr c
  jr nc, bRegIt2
  ld a, b
  ld (=bRegSmc2+2), a
.bRegSmc2
  ld a, (ix+0)
  ld (de), a
  inc de
  ld a, 39
  ld (de), a
  inc de
.bRegIt2
  inc b
  ld a, b
  ret

.regNames
db 'f'
db 'a'
db 'c'
db 'b'
db 'e'
db 'd'
db 'l'
db 'h'


.errorPre
"regs: "	
.errorMsg
ds 21, 0

; Compares the current registers
; to the saved registers and
; stores a 3 byte bit mask
; identifying the changed regs
.TstCmpRegs
  call saveCurRegs
  push af
  push bc
  push de
  push hl

  xor a
  ld hl, changed1
  ld (hl), a
  inc hl
  ld (hl), a
  inc hl
  ld (hl), a

  ld hl, =regEnd-1
  ld bc, =curRegEnd-1
  ld e, 2
  ld d, $2
.block4Check
  ld a, (bc)
  cpd
  jr z, block4Z
  ld a, (changed3)
  or d
  ld (changed3), a
.block4Z
  srl d
  dec e
  jr nz, block4Check

  ld e, 6
  ld d, $80
.block3Check
  ld a, (bc)
  cpd
  jr z, block3Z
  call setChanged2
.block3Z
  srl d
  dec e
  jr nz, block3Check

  ld e, 2
.indexLoop
  ld a, (bc)
  cpd
  call nz, setChanged2
  ld a, (bc)
  cpd
  call nz, setChanged2
  srl d
  dec e
  jr nz, indexLoop

  ld e, 8
  ld d, $80
.block1Check
  ld a, (bc)
  cpd
  jr z, block1Z
  ld a, (changed1)
  or d
  ld (changed1), a
.block1Z
  srl d
  dec e
  jr nz, block1Check

  pop hl
  pop de
  pop bc
  pop af
  ret

.setChanged2
  ld a, (changed2)
  or d
  ld (changed2), a
  ret


; af
; bc
; de
; hl
; ix
; iy
; af'
; bc'
; de'
; hl'
.TstREGS
ds 20, 0
.regEnd
ds 20, 0
.curRegEnd

