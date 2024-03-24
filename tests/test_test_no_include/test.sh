#!/bin/bash

# We're testing here that the test files in the module
# directory do result in a test binary when the main binary
# includes module.

set -e
rm main 2>/dev/null 1>&2 || true
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

if [ -f main.tst ]; then
    echo "main.tst found but not expected"
    exit 1
fi

rm main *.x module/*.x module/*.t
