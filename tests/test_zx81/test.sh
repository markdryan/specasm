#!/bin/bash

set -e
rm zx81 2>/dev/null 1>&2 || true
rm *.x 2>/dev/null 1>&2 || true

../../saimport *.s
../../salink 2>/dev/null 1>&2

str=`od -An -j0 -t x1 -N25 zx81 | xargs`
if [ "$str" != "0b 0c 0d 0e 10 11 13 12 26 3f 26 3f 1c 25 10 11 0e 14 15 16 17 18 19 1a 1b" ]; then
    echo "Expected 0b 0c 0d 0e 10 11 13 12 26 3f 26 3f 1c 25 10 11 0e 14 15 16 17 18 19 1a 1b"
    echo "got      $str"
    exit 1
fi

str=`od -An -j25 -t x1 -N4 zx81 | xargs`
if [ "$str" != "26 26 26 26" ]; then
    echo "Expected 26 26 26 26"
    echo "got      $str"
    exit 1
fi

# A normal number should not be converted, just a character
str=`od -An -j29 -t x1 -N4 zx81 | xargs`
if [ "$str" != "41 41 41 41" ]; then
    echo "Expected 41 41 41 41"
    echo "got      $str"
    exit 1
fi

base=34
for i in {0..15}; do
    offset=$(( base + (i * 2) ))
    str=`od -An -j$offset -t x1 -N1 zx81 | xargs`
    if [ "$str" != "26" ]; then
        echo "Expected 26 at offset $offset"
        echo "got      $str"
        exit 1
    fi
done


base=66
for i in {0..3}; do
    offset=$(( base + (i * 3) ))
    str=`od -An -j$offset -t x1 -N1 zx81 | xargs`
    if [ "$str" != "26" ]; then
        echo "Expected 26 at offset $offset"
        echo "got      $str"
        exit 1
    fi
done

base=78
for i in {0..15}; do
    offset=$(( base + (i * 2) ))
    str=`od -An -j$offset -t x1 -N1 zx81 | xargs`
    if [ "$str" != "41" ]; then
        echo "Expected 41 at offset $offset"
        echo "got      $str"
        exit 1
    fi
done

base=110
for i in {0..3}; do
    offset=$(( base + (i * 3) ))
    str=`od -An -j$offset -t x1 -N1 zx81 | xargs`
    if [ "$str" != "41" ]; then
        echo "Expected 41 at offset $offset"
        echo "got      $str"
        exit 1
    fi
done

base=122
for i in {0..15}; do
    offset=$(( base + (i * 2) ))
    str=`od -An -j$offset -t x1 -N1 zx81 | xargs`
    if [ "$str" != "41" ]; then
        echo "Expected 41 at offset $offset"
        echo "got      $str"
        exit 1
    fi
done

base=154
for i in {0..3}; do
    offset=$(( base + (i * 3) ))
    str=`od -An -j$offset -t x1 -N1 zx81 | xargs`
    if [ "$str" != "41" ]; then
        echo "Expected 41 at offset $offset"
        echo "got      $str"
        exit 1
    fi
done

base=168
for i in {0..1}; do
    offset=$(( base + (i * 4) ))
    str=`od -An -j$offset -t x1 -N1 zx81 | xargs`
    if [ "$str" != "26" ]; then
        echo "Expected 26 at offset $offset"
        echo "got      $str"
        exit 1
    fi
done

base=175
for i in {0..1}; do
    offset=$(( base + (i * 4) ))
    str=`od -An -j$offset -t x1 -N1 zx81 | xargs`
    if [ "$str" != "26" ]; then
        echo "Expected 26 at offset $offset"
        echo "got      $str"
        exit 1
    fi
done

base=184
for i in {0..1}; do
    offset=$(( base + (i * 4) ))
    str=`od -An -j$offset -t x1 -N1 zx81 | xargs`
    if [ "$str" != "41" ]; then
        echo "Expected 41 at offset $offset"
        echo "got      $str"
        exit 1
    fi
done

base=191
for i in {0..1}; do
    offset=$(( base + (i * 4) ))
    str=`od -An -j$offset -t x1 -N1 zx81 | xargs`
    if [ "$str" != "41" ]; then
        echo "Expected 41 at offset $offset"
        echo "got      $str"
        exit 1
    fi
done

base=199
for i in {0..1}; do
    offset=$(( base + (i * 4) ))
    str=`od -An -j$offset -t x1 -N1 zx81 | xargs`
    if [ "$str" != "41" ]; then
        echo "Expected 41 at offset $offset"
        echo "got      $str"
        exit 1
    fi
done

base=205
str=`od -An -j$base -t x1 -N11 zx81 | xargs`
if [ "$str" != "26 26 27 28 29 26 00 26 00 27 00" ]; then
    echo "Expected 26 26 27 28 29 26 00 26 00 27 00"
    echo "got      $str"
    exit 1
fi

base=216
str=`od -An -j$base -t x1 -N6 zx81 | xargs`
if [ "$str" != "41 41 00 41 41 00" ]; then
    echo "Expected 41 41 00 41 41 00"
    echo "got      $str"
    exit 1
fi

base=223
for i in {0..3}; do
    offset=$(( base + (i * 3) ))
    str=`od -An -j$offset -t x1 -N2 zx81 | xargs`
    if [ "$str" != "82 40" ]; then
        echo "Expected 82 40 at offset $offset"
        echo "got      $str"
        exit 1
    fi
done

base=236
for i in {0..1}; do
    offset=$(( base + (i * 4) ))
    str=`od -An -j$offset -t x1 -N2 zx81 | xargs`
    if [ "$str" != "82 40" ]; then
        echo "Expected 82 40 at offset $offset"
        echo "got      $str"
        exit 1
    fi
done

offset=242
str=`od -An -j$offset -t x1 -N1 zx81 | xargs`
if [ "$str" != "00" ]; then
    echo "Expected 00 at offset $offset"
    echo "got      $str"
    exit 1
fi

offset=243
str=`od -An -j$offset -t x1 -N2 zx81 | xargs`
if [ "$str" != "82 40" ]; then
    echo "Expected 82 40 at offset $offset"
    echo "got      $str"
    exit 1
fi

offset=245
str=`od -An -j$offset -t x1 -N4 zx81 | xargs`
if [ "$str" != "dd 36 01 41" ]; then
    echo "Expected dd 36 01 41 at offset $offset"
    echo "got      $str"
    exit 1
fi

offset=249
str=`od -An -j$offset -t x1 -N4 zx81 | xargs`
if [ "$str" != "fd 36 01 41" ]; then
    echo "Expected fd 36 01 41 at offset $offset"
    echo "got      $str"
    exit 1
fi
offset=253
str=`od -An -j$offset -t x1 -N1 zx81 | xargs`
if [ "$str" != "26" ]; then
    echo "Expected 26 at offset $offset"
    echo "got      $str"
    exit 1
fi

rm zx81
rm *.x

