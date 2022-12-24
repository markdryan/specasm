#!/bin/bash

set -e
rm equ 2>/dev/null 1>&2 || true
rm *.x 2>/dev/null 1>&2 || true

../../saimport *.s
../../salink 2>/dev/null 1>&2

# adc a, =Test2
word=`od -An -j0 -tx1 -N2 equ | xargs`
if [ "$word" != "ce a8" ]; then
    echo "adc a, =Test2"
    exit 1
fi

# add a, =divide
word=`od -An -j2 -tx1 -N2 equ | xargs`
if [ "$word" != "c6 03" ]; then
    echo "add a, =divide"
    exit 1
fi

# and =unary+9
word=`od -An -j4 -tx1 -N2 equ | xargs`
if [ "$word" != "e6 ff" ]; then
    echo "and =unary+9"
    exit 1
fi

# ld hl, =later-Main+1
triple=`od -An -j6 -tx1 -N3 equ | xargs`
if [ "$triple" != "21 07 00" ]; then
    echo "ld hl, =later-Main+1"
    exit 1
fi

# ld hl, =later-Main+Element_count
triple=`od -An -j9 -tx1 -N3 equ | xargs`
if [ "$triple" != "21 11 00" ]; then
    echo "ld hl, =later-Main+Element_count"
    exit 1
fi

# bit =7, l
word=`od -An -j12 -tx1 -N2 equ | xargs`
if [ "$word" != "cb 7d" ]; then
    echo "bit =7, l"
    exit 1
fi

# res =6, l
word=`od -An -j14 -tx1 -N2 equ | xargs`
if [ "$word" != "cb b5" ]; then
    echo "res =6, l"
    exit 1
fi

# set =5, l
word=`od -An -j16 -tx1 -N2 equ | xargs`
if [ "$word" != "cb ed" ]; then
    echo "set =5, l"
    exit 1
fi

# call =$dead
triple=`od -An -j18 -tx1 -N3 equ | xargs`
if [ "$triple" != "cd ad de" ]; then
    echo "call =\$dead"
    exit 1
fi

# jp nc, =$beef
triple=`od -An -j21 -tx1 -N3 equ | xargs`
if [ "$triple" != "d2 ef be" ]; then
    echo "jp nc, =$\beef"
    exit 1
fi

# cp =Size-1
word=`od -An -j24 -tx1 -N2 equ | xargs`
if [ "$word" != "fe 0f" ]; then
    echo "cp =Size-1"
    exit 1
fi

# im =0
byte=`od -An -j27 -tx1 -N1 equ | xargs`
if [ "$byte" != "46" ]; then
    echo "im =0"
    exit 1
fi

# im =1
byte=`od -An -j29 -tx1 -N1 equ | xargs`
if [ "$byte" != "56" ]; then
    echo "im =1"
    exit 1
fi

# im =2
byte=`od -An -j31 -tx1 -N1 equ | xargs`
if [ "$byte" != "5e" ]; then
    echo "im =2"
    exit 1
fi

# in a, (=$1+Element_count)
word=`od -An -j32 -tx1 -N2 equ | xargs`
if [ "$word" != "db 0c" ]; then
    echo "in a, (=\$1+Element_count)"
    exit 1
fi

# out (=$1+Element_count), a
word=`od -An -j34 -tx1 -N2 equ | xargs`
if [ "$word" != "d3 0c" ]; then
    echo "out (=$1+Element_count), a"
    exit 1
fi

# or =complement
word=`od -An -j36 -tx1 -N2 equ | xargs`
if [ "$word" != "f6 ef" ]; then
    echo "or =complement"
    exit 1
fi

# rst =Size
byte=`od -An -j38 -tx1 -N1 equ | xargs`
if [ "$byte" != "d7" ]; then
    echo "rst =Size"
    exit 1
fi

# ld a, =Size
byte=`od -An -j40 -tx1 -N1 equ | xargs`
if [ "$byte" != "10" ]; then
    echo "ld a, =Size"
    exit 1
fi

# ld b, =unary
word=`od -An -j41 -tx1 -N2 equ | xargs`
if [ "$word" != "06 f6" ]; then
    echo "ld b, =unary"
    exit 1
fi

# ld c, =logical
word=`od -An -j43 -tx1 -N2 equ | xargs`
if [ "$word" != "0e 06" ]; then
    echo "ld c, =logical"
    exit 1
fi

# ld d, =1 << 5
word=`od -An -j45 -tx1 -N2 equ | xargs`
if [ "$word" != "16 20" ]; then
    echo "ld d, =1 << 5"
    exit 1
fi

# ld e, =(33 & 1)
word=`od -An -j47 -tx1 -N2 equ | xargs`
if [ "$word" != "1e 01" ]; then
    echo "ld e, =(33 & 1)"
    exit 1
fi

# ld h, =left_shift
word=`od -An -j49 -tx1 -N2 equ | xargs`
if [ "$word" != "26 2c" ]; then
    echo "ld h, =left_shift"
    exit 1
fi

# ld l, =right_shift
word=`od -An -j51 -tx1 -N2 equ | xargs`
if [ "$word" != "2e 02" ]; then
    echo "ld l, =right_shift"
    exit 1
fi

# ld hl, =$dead
triple=`od -An -j53 -tx1 -N3 equ | xargs`
if [ "$triple" != "21 ad de" ]; then
    echo "ld hl, =\$dead"
    exit 1
fi

# ld de, =precedence
triple=`od -An -j56 -tx1 -N3 equ | xargs`
if [ "$triple" != "11 0f 00" ]; then
    echo "ld de, =precedence"
    exit 1
fi

# ld bc, =$bac0
triple=`od -An -j59 -tx1 -N3 equ | xargs`
if [ "$triple" != "01 c0 ba" ]; then
    echo "ld bc, =\$bac0"
    exit 1
fi

# ld sp, =$11+2
triple=`od -An -j62 -tx1 -N3 equ | xargs`
if [ "$triple" != "31 13 00" ]; then
    echo "ld sp, =\$11+2"
    exit 1
fi

# ld hl, (=$8000+1)
triple=`od -An -j65 -tx1 -N3 equ | xargs`
if [ "$triple" != "2a 01 80" ]; then
    echo "ld hl, (=\$8000+1)"
    exit 1
fi

# ld a, (=\$8000+Size)
triple=`od -An -j68 -tx1 -N3 equ | xargs`
if [ "$triple" != "3a 10 80" ]; then
    echo "ld a, (=\$8000+Size)"
    exit 1
fi

# ld (=$8000 - $a), hl
triple=`od -An -j71 -tx1 -N3 equ | xargs`
if [ "$triple" != "22 f6 7f" ]; then
    echo "ld (=\$8000 - \$a), hl"
    exit 1
fi

# ld (=$8000 - $b), a
triple=`od -An -j74 -tx1 -N3 equ | xargs`
if [ "$triple" != "32 f5 7f" ]; then
    echo "ld (=\$8000 - \$b), a"
    exit 1
fi

# ld (hl), =$80+1
word=`od -An -j77 -tx1 -N2 equ | xargs`
if [ "$word" != "36 81" ]; then
    echo "ld (hl), =\$80+1"
    exit 1
fi

# ld ix, =Test
dword=`od -An -j79 -tx1 -N4 equ | xargs`
if [ "$dword" != "dd 21 b0 00" ]; then
    echo "ld ix, =Test"
    exit 1
fi

# ld ix, (=Test)
dword=`od -An -j83 -tx1 -N4 equ | xargs`
if [ "$dword" != "dd 2a b0 00" ]; then
    echo "ld ix, (=Test)"
    exit 1
fi

# ld (=Test), ix
dword=`od -An -j87 -tx1 -N4 equ | xargs`
if [ "$dword" != "dd 22 b0 00" ]; then
    echo "ld (=Test), ix"
    exit 1
fi

# ld iy, =Test
dword=`od -An -j91 -tx1 -N4 equ | xargs`
if [ "$dword" != "fd 21 b0 00" ]; then
    echo "ld iy, =Test"
    exit 1
fi

# ld iy, (=Test)
dword=`od -An -j95 -tx1 -N4 equ | xargs`
if [ "$dword" != "fd 2a b0 00" ]; then
    echo "ld iy, (=Test)"
    exit 1
fi

# ld (=Test), iy
dword=`od -An -j99 -tx1 -N4 equ | xargs`
if [ "$dword" != "fd 22 b0 00" ]; then
    echo "ld (=Test), iy"
    exit 1
fi

# ld de, (=Test)
dword=`od -An -j103 -tx1 -N4 equ | xargs`
if [ "$dword" != "ed 5b b0 00" ]; then
    echo "ld de, (=Test)"
    exit 1
fi

# ld (=Test), de
dword=`od -An -j107 -tx1 -N4 equ | xargs`
if [ "$dword" != "ed 53 b0 00" ]; then
    echo "ld (=Test), de"
    exit 1
fi

# ld bc, (=Test)
dword=`od -An -j111 -tx1 -N4 equ | xargs`
if [ "$dword" != "ed 4b b0 00" ]; then
    echo "ld bc, (=Test)"
    exit 1
fi

# ld (=Test), bc
dword=`od -An -j115 -tx1 -N4 equ | xargs`
if [ "$dword" != "ed 43 b0 00" ]; then
    echo "ld (=Test), bc"
    exit 1
fi

# ld sp, (=Test)
dword=`od -An -j119 -tx1 -N4 equ | xargs`
if [ "$dword" != "ed 7b b0 00" ]; then
    echo "ld sp, (=Test)"
    exit 1
fi

# ld (=Test), sp
dword=`od -An -j123 -tx1 -N4 equ | xargs`
if [ "$dword" != "ed 73 b0 00" ]; then
    echo "ld (=Test), sp"
    exit 1
fi

# sbc a, =long
word=`od -An -j127 -tx1 -N2 equ | xargs`
if [ "$word" != "de 14" ]; then
    echo "sbc a, =long"
    exit 1
fi

# sub =divide
word=`od -An -j129 -tx1 -N2 equ | xargs`
if [ "$word" != "d6 03" ]; then
    echo "sub =divide"
    exit 1
fi

# xor =logical
word=`od -An -j131 -tx1 -N2 equ | xargs`
if [ "$word" != "ee 06" ]; then
    echo "xor =logical"
    exit 1
fi

# db =$80+1
byte=`od -An -j133 -tx1 -N1 equ | xargs`
if [ "$byte" != "81" ]; then
    echo "db =$80+1"
    exit 1
fi

# dw =256
word=`od -An -j134 -tx1 -N2 equ | xargs`
if [ "$word" != "00 01" ]; then
    echo "dw =256"
    exit 1
fi

rm equ
rm *.x

