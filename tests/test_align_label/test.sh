#!/bin/bash

set -e
rm align 2>/dev/null 1>&2 || true
rm *.x 2>/dev/null 1>&2 || true

../../saimport *.s
../../salink 2>/dev/null 1>&2
offset=`od -An -j1 -t x1 -N2 align | xargs`
if [ "$offset" != "00 81" ]; then
    exit 1
fi

offset=`od -An -j4 -t x1 -N2 align | xargs`
if [ "$offset" != "08 81" ]; then
    exit 1
fi

offset=`od -An -j212 -t x1 -N2 align | xargs`
if [ "$offset" != "08 81" ]; then
    exit 1
fi


rm align
rm *.x

