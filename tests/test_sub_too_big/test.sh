#!/bin/bash

set -e
rm subtraction 2>/dev/null 1>&2 || true

if ../../salink 2>/dev/null 1>&2 ; then
    exit 1
fi
