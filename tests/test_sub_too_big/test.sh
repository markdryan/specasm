#!/bin/bash

set -e

if [ -z "${SPECASM_TARGET_NEXT_OPCODES}" ]; then
    exit 0
fi

rm subtraction 2>/dev/null 1>&2 || true

if ../../salink 2>/dev/null 1>&2 ; then
    exit 1
fi
