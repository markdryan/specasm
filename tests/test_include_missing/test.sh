#!/bin/bash

set -e
rm *.x 2>/dev/null 1>&2 || true


../../saimport *.s
if ../../salink 2>/dev/null 1>&2; then
    exit 1
fi
rm *.x

