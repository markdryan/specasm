#!/bin/bash

set -e
rm align 2>/dev/null 1>&2 || true
rm *.x 2>/dev/null 1>&2 || true

../../saimport *.s
../../salink 2>/dev/null 1>&2
hello=`od -An -j73 -a -N5 align | xargs`
if [ "$hello" != "h e l l o" ]; then
    exit 1
fi

goodbye=`od -An -j81 -a -N7 align | xargs`
if [ "$goodbye" != "g o o d b y e" ]; then
    exit 1
fi

flushed=`od -An -j1097 -a -N7 align | xargs`
if [ "$flushed" != "f l u s h e d" ]; then
    exit 1
fi


rm align
rm *.x

