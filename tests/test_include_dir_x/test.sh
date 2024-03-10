#!/bin/bash

set -e
rm main 2>/dev/null 1>&2 || true
find . -name '*.x' -exec rm {} \; 2>/dev/null 1>&2 || true


../../saimport *.s
pushd sub.x > /dev/null
../../../saimport *.s
popd > /dev/null
../../salink  2>/dev/null 1>&2

# First byte should be 1
byte=`od -An -j0 -tx1 -N1 main | xargs`
if [ "$byte" != "01" ]; then
    echo "01 expected got $byte"
    exit 1
fi

# Second byte should be 2
byte=`od -An -j1 -tx1 -N1 main | xargs`
if [ "$byte" != "02" ]; then
    echo "02 expected got $byte"
    exit 1
fi

rm main
find . -name '*.x' -exec rm {} \; 2>/dev/null 1>&2 || true

