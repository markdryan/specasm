org $8000
.Main
ld a, small_start - small_end
.small_start
ds 256, 0
.small_end

