#!/bin/bash

. ../utils/unit.sh

set -e
rm main main.tst 2>/dev/null 1>&2 || true
rm *.x *.t 2>/dev/null 1>&2 || true

../../saimport *.ts *.s
../../salink 2>/dev/null 1>&2

expected_test_names="Gooder Bad"
test_names=$( dump_test_names main.tst 32768 )
if [ "$test_names" != "$expected_test_names" ]; then
    echo "Expected \"$expected_test_names\" got \"$test_names"\"
    exit 1
fi

if [ ! -f main.map ]; then
    echo "main.map not found"
    exit 1
fi

if [ ! -f main.tmt ]; then
    echo "main.tmt not found"
    exit 1
fi

../../samake tst 2>/dev/null 1>&2
if [ ! -f unit.bas ]; then
    echo "unit.bas not found"
    exit 1
fi

rm main main.tst main.map main.tmt unit.bas
rm *.x *.t
