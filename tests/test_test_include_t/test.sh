#!/bin/bash

. ../utils/unit.sh

# We're testing here that we can explicitly import a .t file.

set -e
rm main.tst main 2>/dev/null 1>&2 || true
rm *.x *.t module/*.x module/*.t 2>/dev/null 1>&2 || true

../../saimport *.s
pushd module 2>/dev/null 1>&2
../../../saimport *.s *.ts
popd 2>/dev/null 1>&2
../../salink 2>/dev/null 1>&2

if [ ! -f main ]; then
    echo "main not found"
    exit 1
fi

if [ ! -f main.tst ]; then
    echo "main.tst not found"
    exit 1
fi

test_names=$( dump_test_names main.tst 32768)
if [ $test_names != "TestFunction" ]; then
    echo "Expected TestFunction got $test_names"
    exit 1
fi

rm main.tst main *.x module/*.x module/*.t
