#!/bin/bash

set -e
rm backward 2>/dev/null 1>&2 || true
rm *.x 2>/dev/null 1>&2 || true

../../saimport *.s
../../salink 2>/dev/null 1>&2
absolute=`od -An -j380 -tx1 -N3 backward | xargs`
if [ "$absolute" != "c2 00 80" ]; then
    exit 1
fi
relative=`od -An -j383 -tx1 -N2 backward | xargs`
if [ "$relative" != "18 80" ]; then
    exit 1
fi

rm backward
rm *.x


