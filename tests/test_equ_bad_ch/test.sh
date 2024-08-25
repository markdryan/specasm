#!/bin/bash

set -e

declare -a arr=("test_1" "test_2" "test_3")
for i in "${arr[@]}"
do
    cd $i
    ../../../saimport *.s
    if ../../salink 2>/dev/null 1>&2 ; then
	rm *.x
	exit 1
    fi
    rm *.x
    cd ..
done

