.notletter equ 65

.Main
zx81
'"#$:()<>azAZ09():=+-*/;,.'
ds 4, 'A'
ds 4, 65

adc a, 'A'
add a, 'A'
and 'A'
cp  'A'
ld (hl), 'A'
ld a, 'A'
ld b, 'A'
ld c , 'A'
ld d, 'A'
ld e, 'A'
ld h, 'A'
ld l, 'A'
or 'A'
sbc a, 'A'
sub 'A'
xor 'A'

ld bc, 'A'
ld de, 'A'
ld hl, 'A'
ld sp, 'A'

adc a, 65
add a, 65
and 65
cp  65
ld (hl), 65
ld a, 65
ld b, 65
ld c , 65
ld d, 65
ld e, 65
ld h, 65
ld l, 65
or 65
sbc a, 65
sub 65
xor 65

ld bc, 65
ld de, 65
ld hl, 65
ld sp, 65

adc a, =notletter
add a, =notletter
and =notletter
cp  =notletter
ld (hl), =notletter
ld a, =notletter
ld b, =notletter
ld c , =notletter
ld d, =notletter
ld e, =notletter
ld h, =notletter
ld l, =notletter
or =notletter
sbc a, =notletter
sub =notletter
xor =notletter

ld bc, =notletter
ld de, =notletter
ld hl, =notletter
ld sp, =notletter

ld (ix+1), 'A'
ld (iy+1), 'A'
ld ix, 'A'
ld iy, 'A'

ld (ix+1), $41
ld (iy+1), $41
ld ix, $41
ld iy, $41

ld ix,  =notletter
ld iy,  =notletter

db 'A'
db 'A', 'B', 'C', 'D'
dw 'A'
dw 'A', 'B'
db 65
dw $41
db =notletter
dw =notletter

ld bc, Main
ld de, Main
ld hl, Main
ld sp, Main
ld ix, Main
ld iy, Main

db ' '
dw Main
