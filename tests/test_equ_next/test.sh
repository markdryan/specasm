#!/bin/bash

if [ -z "${SPECASM_TARGET_NEXT_OPCODES}" ]; then
    exit 0
fi

set -e
rm equ 2>/dev/null 1>&2 || true
rm *.x 2>/dev/null 1>&2 || true

../../saimport *.s
../../salink 2>/dev/null 1>&2

# test =Size
word=`od -An -j2 -tx1 -N1 equ | xargs`
if [ "$word" != "10" ]; then
    echo "test =Size"
    exit 1
fi

word=`od -An -j5 -tx1 -N1 equ | xargs`
if [ "$word" != "10" ]; then
    echo "nextreg =Size, a"
    exit 1
fi

word=`od -An -j8 -tx1 -N1 equ | xargs`
if [ "$word" != "10" ]; then
    echo "nextreg =Size, 10"
    exit 1
fi

word=`od -An -j12 -tx1 -N2 equ | xargs`
if [ "$word" != "02 01" ]; then
    echo "add hl, =(Size * 16) + 2"
    exit 1
fi

word=`od -An -j16 -tx1 -N2 equ | xargs`
if [ "$word" != "03 01" ]; then
    echo "add bc, =(Size * 16) + 2"
    exit 1
fi

word=`od -An -j20 -tx1 -N2 equ | xargs`
if [ "$word" != "04 02" ]; then
    echo "add de, =(Size * 32) + 4"
    exit 1
fi

word=`od -An -j24 -tx1 -N2 equ | xargs`
if [ "$word" != "01 10" ]; then
    echo "push =(Size*17)"
    exit 1
fi


rm equ
rm *.x

