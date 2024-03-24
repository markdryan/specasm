.check
  ld a, (de)
  cpi
  jr nz, fail
  inc de
  ld a, c
  or a
  jr nz, check
  ret
.fail
  ld bc, 1
  ret

.TestGooder
  ld hl, good
  ld b, =good_end-good
  ld c, 0
  call ToLower
  ld hl, goodexp
  ld de, good
  ld c, =good_end-good
  ld b, 0
  jr check

.TestBad
  ld hl, bad
  ld b, =bad_end-bad
  ld c, 0
  call ToLower
  ld hl, badexp
  ld de, bad
  ld c, =bad_end-bad
  ld b, 0
  jr check


.good
"Hello World"
.good_end
.goodexp
"hello world"

.bad
"Hello World["
.bad_end
.badexp
"hello world["
.END
