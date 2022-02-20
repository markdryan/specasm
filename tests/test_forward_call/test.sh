#!/bin/bash

set -e
rm forward 2>/dev/null 1>&2 || true
rm *.x 2>/dev/null 1>&2 || true

../../saimport *.s
../../salink 2>/dev/null 1>&2
absolute=`od -An -tx1 -N3 forward | xargs`
if [ "$absolute" != "cd 30 81" ]; then
    exit 1
fi

rm *.x


