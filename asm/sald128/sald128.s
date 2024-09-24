.timer equ $5c78
.cpuid equ 2899
.bankVar equ $5b5c
.bankPort equ $7ffd
.Main
; jump table
dw calib
dw bank0
dw bank4
dw bank6
dw bank1

.calib
  halt
  ld hl, 0
  ld a, (=timer)
  ld b, a
.calibLoop
  inc hl
  nop
  nop
  nop
  nop
  ld a, (=timer)
  cp b
  jr z, calibLoop
  ld c, l
  ld b, h
  or a
  ret

  align 4
.bank128
db 16,20,22,17
.bankPlus2a
db 16,17,19,20

.bank0
  ld c, 0
  jr switchBank
.bank4
  ld c, 1
  jr switchBank
.bank6
  ld c, 2
  jr switchBank
.bank1
  ld c, 3


.switchBank
  ld a, (=cpuid)
  cp 126
  jr nz, toastPages
  ld hl, bankPlus2a
  jr startSwitch
.toastPages
  ld hl, bank128
.startSwitch
  ld a, c
  add a, l
  ld l, a
  ld b, (hl)
  di
  ld a, (=bankVar)
  and $E0
  or b
  ld (=bankVar), a
  ld bc, =bankPort
  out (c), a
  ei
  ld bc, 0
  or a
  ret

