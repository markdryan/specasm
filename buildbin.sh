#!/bin/bash

set -e

script_dir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
build_dir=$1

# Build the posix version of the tools with the correct flags.
# We'll need these for assembling and linking the .x file and
# binaries included in the release.

pushd $script_dir
make clean
make -j
popd

# Build sald128.s

if [ "$2" == "128" ]; then
    rm -rf $build_dir/sald128
    cp -r $script_dir/asm/sald128 $build_dir
    pushd $build_dir/sald128
    $script_dir/saimport *.s
    $script_dir/salink
    popd
fi
    
