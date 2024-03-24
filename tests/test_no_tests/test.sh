#!/bin/bash

set -e
rm *.x *.t 2>/dev/null 1>&2 || true

../../saimport *.ts *.s
if ../../salink 2>/dev/null 1>&2 ; then
    echo "Link should have failed as there are no tests"
    exit 1
fi

 rm *.x *.t 2>/dev/null 1>&2 || true



