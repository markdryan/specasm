#!/bin/bash

set -e
rm label12 2>/dev/null 1>&2 || true
rm *.x 2>/dev/null 1>&2 || true

../../saimport *.s
../../salink 2>/dev/null 1>&2

byte=`od -An -j0 -tx1 -N1 label12 | xargs`
if [ "$byte" != "13" ]; then
    exit 1
fi

rm label12
rm *.x
