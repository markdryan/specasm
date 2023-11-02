org $8000
.Main
.Size equ 16

test =Size
nextreg =Size, a
nextreg =Size, 10
add hl, =(Size * 16) + 2
add bc, =(Size * 16) + 3
add de, =(Size * 32) + 4
push =(Size*17)
