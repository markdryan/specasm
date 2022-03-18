org $8000
.Main
ld hl, small_end - small_start
ld hl, big_end - big_start
ld a, small_end - small_start
dw small_end - small_start
dw big_end - big_start
db small_end - small_start
.small_start
ds 255, 0
.small_end
.big_start
ds 1024, 0
.big_end

