#!/bin/bash

set -e
rm inc_bin data 2>/dev/null 1>&2 || true
rm *.x 2>/dev/null 1>&2 || true

cat /dev/zero | head -c 2049 | tr '\000' '\177' > data

../../saimport *.s
../../salink 2>/dev/null 1>&2

# First alignment byte
byte=`od -An -j5 -tx1 -N1 inc_bin | xargs`
if [ "$byte" != "00" ]; then
    echo "Expected 0 byte from alignment"
    exit 1
fi

# jp local target
word=`od -An -j2 -tx1 -N2 inc_bin | xargs`
if [ "$word" != "0a 88" ]; then
    echo "Expected jump target 880a"
    exit 1
fi

# first file byte
byte=`od -An -j8 -tx1 -N1 inc_bin | xargs`
if [ "$byte" != "7f" ]; then
    echo "Expected 7f at byte 8"
    exit 1
fi

# last file byte
byte=`od -An -j2056 -tx1 -N1 inc_bin | xargs`
if [ "$byte" != "7f" ]; then
    echo "Expected 7f at byte 2056"
    exit 1
fi

# db 'B'
byte=`od -An -j2057 -tx1 -N1 inc_bin | xargs`
if [ "$byte" != "42" ]; then
    echo "db 'B'"
    exit 1
fi

# target of jp Global
word=`od -An -j2060 -tx1 -N2 inc_bin | xargs`
if [ "$word" != "04 80" ]; then
    echo "Expected jump target of 8004"
    exit 1
fi

# target of jp Global2
word=`od -An -j2063 -tx1 -N2 inc_bin | xargs`
if [ "$word" != "0a 88" ]; then
    echo "Expected jump target 880a"
    exit 1
fi

rm inc_bin
rm data
rm *.x

