#!/bin/bash

set -e
rm main 2>/dev/null 1>&2 || true
find . -name '*.x' -exec rm {} \; 2>/dev/null 1>&2 || true


../../saimport *.s
pushd 01_sub > /dev/null
../../../saimport *.s
pushd 01_sub.x > /dev/null
../../../../saimport *.s
popd > /dev/null
popd > /dev/null
../../salink  2>/dev/null 1>&2

# First byte should be 1
byte=`od -An -j0 -tx1 -N1 main | xargs`
if [ "$byte" != "01" ]; then
    echo "Main object not first"
    exit 1
fi

# Second byte should be 3
byte=`od -An -j1 -tx1 -N1 main | xargs`
if [ "$byte" != "03" ]; then
    echo "Expected 03 got $byte"
    exit 1
fi

rm main
find . -name '*.x' -exec rm {} \; 2>/dev/null 1>&2 || true

