map
.Main
  ld hl, demo
  ld b, =demo_end-demo
  call ToLower
  ld hl, demo
  ld b, =demo_end-demo
.loop
  ld a, (hl)
  rst 16
  inc hl
  djnz loop
  ret

.ToLower
  ld a, (hl)
  or 32
  ld (hl), a
  inc hl
  djnz ToLower
  ret

.demo
"HELLO WORLD"
.demo_end
