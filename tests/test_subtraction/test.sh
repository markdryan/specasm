#!/bin/bash

set -e

if [ -z "${SPECASM_TARGET_NEXT_OPCODES}" ]; then
    exit 0
fi

rm subtraction 2>/dev/null 1>&2 || true

../../salink 2>/dev/null 1>&2
diff=`od -An -tx1 -N3 subtraction | xargs`
if [ "$diff" != "21 ff 00" ]; then
    exit 1
fi
diff=`od -An -tx1 -j3 -N3 subtraction | xargs`
if [ "$diff" != "21 00 04" ]; then
    exit 1
fi
diff=`od -An -tx1 -j6 -N2 subtraction | xargs`
if [ "$diff" != "3e ff" ]; then
    exit 1
fi
diff=`od -An -tx1 -j8 -N2 subtraction | xargs`
if [ "$diff" != "ff 00" ]; then
    exit 1
fi
diff=`od -An -tx1 -j10 -N2 subtraction | xargs`
if [ "$diff" != "00 04" ]; then
    exit 1
fi
diff=`od -An -tx1 -j12 -N1 subtraction | xargs`
if [ "$diff" != "ff" ]; then
    exit 1
fi

rm subtraction

