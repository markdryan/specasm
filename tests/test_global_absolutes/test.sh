#!/bin/bash

set -e
rm global 2>/dev/null 1>&2 || true
rm *.x 2>/dev/null 1>&2 || true

../../saimport *.s
../../salink 2>/dev/null 1>&2
absolute=`od -An -tx1 -N3 global | xargs`
if [ "$absolute" != "21 09 80" ]; then
    exit 1
fi
absolute=`od -An -tx1 -j 3 -N3 global | xargs`
if [ "$absolute" != "2a 09 80" ]; then
    exit 1
fi
absolute=`od -An -tx1 -j 7 -N2 global | xargs`
if [ "$absolute" != "09 80" ]; then
    exit 1
fi

rm global
rm *.x

