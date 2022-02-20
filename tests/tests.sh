#!/bin/bash

for i in * ; do
    if [ -d "$i" ]; then
	if [ -f "$i/test.sh" ] ; then
	    echo -n $i
	    cd $i
	    if ! ./test.sh; then
		echo ": [FAIL]"
		cd ..
		exit 1
	    else
		echo ": [OK]"
	    fi
	    cd ..
	fi
    fi
done



