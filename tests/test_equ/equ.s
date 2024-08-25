org $8000
.Main
.Size equ 16
.Element_cnt equ 11
.Test equ (Size * Element_cnt)
.unary equ -10
.complement equ ~16
.divide equ 10 /3
.logical equ ( 2 | 4 ) & 15
.precedence equ 10 * 2 - 5
.left_shift equ 11 << 2
.right_shift equ 11 >> 2
.long equ 10+10&15+15-2

adc a, =Test2
add a, =divide
and =unary+9
.later
ld hl, =later-Main+1
ld hl, =later-Main+Element_cnt
bit =7, l
res =6, l
set =5, l
call =$dead
jp nc, =$beef
cp =Size-1
im =0
im =1
im =Size>>3
in a, (=$1+Element_cnt)
out (=$1+Element_cnt), a
or =complement
rst =Size
ld a, =Size
ld b, =unary
ld c, =logical
ld d, =1 << 5
ld e, =(33 & 1)
ld h, =left_shift
ld l, =right_shift
ld hl, =$dead
ld de, =precedence
ld bc, =$bac0
ld sp, =$11+2
ld hl, (=$8000+1)
ld a, (=$8000+Size)
ld (=$8000 - $a), hl
ld (=$8000 - $b), a
ld (hl), =$80+1
ld ix, =Test
ld ix, (=Test)
ld (=Test), ix
ld iy, =Test
ld iy, (=Test)
ld (=Test), iy
ld de, (=Test)
ld (=Test), de
ld bc, (=Test)
ld (=Test), bc
ld sp, (=Test)
ld (=Test), sp
sbc a, =long
sub =divide
xor =logical


db =$80+1
dw =256

ld (ix+32), =divide
ld (iy+32), =Size

db ='A'+1

