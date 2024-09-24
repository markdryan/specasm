#!/bin/bash

set -e
rm longname0012 longname0012.map 2>/dev/null 1>&2 || true
rm *.x 2>/dev/null 1>&2 || true

../../saimport *.s
../../salink 2>/dev/null 1>&2

if [ ! -f longname0012 ]; then
    echo "longname0012 not found"
    exit 1
fi

if [ ! -f longname0012.map ]; then
    echo "longname0012.map not found"
    exit 1
fi

rm longname0012 longname0012.map
rm *.x

