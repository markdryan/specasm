#!/bin/bash

set -e
rm forward 2>/dev/null 1>&2 || true
rm *.x 2>/dev/null 1>&2 || true

../../saimport *.s
../../salink 2>/dev/null 1>&2
absolute=`od -An -tx1 -N3 forward | xargs`
if [ "$absolute" != "c3 84 81" ]; then
    exit 1
fi
relative=`od -An -j3 -tx1 -N2 forward | xargs`
if [ "$relative" != "18 7e" ]; then
    exit 1
fi

rm forward
rm *.x

