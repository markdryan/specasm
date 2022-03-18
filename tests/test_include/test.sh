#!/bin/bash

set -e
rm main 2>/dev/null 1>&2 || true
rm *.x sub/*.x 2>/dev/null 1>&2 || true


../../saimport *.s
cd sub
../../../saimport *.s
cd ..
../../salink 2>/dev/null 1>&2
offset=`od -An -j2 -t x1 -N2 main | xargs`
if [ "$offset" != "07 80" ]; then
    exit 1
fi

# This relies on the fact that sub.x is included before
# sub2.x, which currently means that the bytes of sub.x
# will get written into the binary before those of sub2.x.

offset=`od -An -j5 -t x1 -N2 main | xargs`
if [ "$offset" != "08 80" ]; then
    exit 1
fi

rm main
rm *.x sub/*.x

