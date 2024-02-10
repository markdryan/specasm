#!/bin/bash

set -e
rm main 2>/dev/null 1>&2 || true
rm *.x 2>/dev/null 1>&2 || true

../../saimport *.s
../../salink 2>/dev/null 1>&2

# First byte should be 1
byte=`od -An -j0 -tx1 -N1 main | xargs`
if [ "$byte" != "01" ]; then
    echo "Main object not first"
    exit 1
fi

# Second byte should be 1
byte=`od -An -j1 -tx1 -N1 main | xargs`
if [ "$byte" != "02" ]; then
    echo "00_data.s is not second"
    exit 1
fi

# Third byte should be 2
byte=`od -An -j2 -tx1 -N1 main | xargs`
if [ "$byte" != "03" ]; then
    echo "01_data.s is not third"
    exit 1
fi

rm main
rm *.x

