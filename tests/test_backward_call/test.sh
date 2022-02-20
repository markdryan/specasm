#!/bin/bash

set -e
rm backward 2>/dev/null 1>&2 || true
rm *.x 2>/dev/null 1>&2 || true

../../saimport *.s
../../salink 2>/dev/null 1>&2
absolute=`od -An -j257 -tx1 -N3 backward | xargs`
if [ "$absolute" != "c4 00 80" ]; then
    exit 1
fi

rm backward
rm *.x


