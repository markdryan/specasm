#!/bin/bash

set -e
rm align_aligned 2>/dev/null 1>&2 || true
rm *.x 2>/dev/null 1>&2 || true

../../saimport *.s
../../salink 2>/dev/null 1>&2
offset=`od -An -j1 -t x1 -N2 align_aligned | xargs`
if [ "$offset" != "03 80" ]; then
    exit 1
fi

offset=`od -An -j5 -t x1 -N2 align_aligned | xargs`
if [ "$offset" != "08 80" ]; then
    exit 1
fi

rm align_aligned
rm *.x

