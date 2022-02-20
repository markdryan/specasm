org &8000
.Main
ld hl, small_end - small_start
ld hl, big_end - big_start
ld a, small_end - small_start
equw small_end - small_start
equw big_end - big_start
equb small_end - small_start
.small_start
repb 0, 255
.small_end
.big_start
repb 0, 1024
.big_end

