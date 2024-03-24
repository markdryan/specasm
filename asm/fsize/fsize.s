  org $EA60
.FSize equ $EA60
ds 32, 0
.Main
  ld a, '*'
  ld ix, =FSize
  ld b, 1
  rst $8
db $9A
  jr c, fail
  ld (handle), a

  ld ix, stat
  rst $8
db $A1
  jr c, closeFail

  ld a, (handle)
  rst $8
db $9B

  ld hl, =stat+7
  ld c, (hl)
  inc hl
  ld b, (hl)
  ret

.closeFail
  ld a, (handle)
  rst $8
db $9B

.fail
  ld bc, 0
  ret
.handle
db 0
.stat
ds 11, 0
