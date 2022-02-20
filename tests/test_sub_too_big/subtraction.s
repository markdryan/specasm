org &8000
.Main
ld a, small_end - small_start
.small_start
repb 0, 256
.small_end

