#!/bin/bash

. ../utils/unit.sh

set -e
rm unit.bas test.tmt test.tst 2>/dev/null 1>&2 || true
rm *.x *.t 2>/dev/null 1>&2 || true

../../saimport *.ts *.s
../../salink 2>/dev/null 1>&2

test_names=$( dump_test_names test.tst 36864)
if [ $test_names != "Global" ]; then
    echo "Expected Global got $test_names"
    exit 1
fi

if [ ! -f test.tmt ]; then
    echo "test.tmt not found"
    exit 1
fi

../../samake tst 2>/dev/null 1>&2
if [ ! -f unit.bas ]; then
    echo "unit.bas not found"
    exit 1
fi

rm unit.bas test.tst test.tmt
rm *.x *.t
