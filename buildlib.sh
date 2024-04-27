#!/bin/bash

set -e

script_dir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
release_dir=$1

if [ "$2" = "next" ]; then
    export CFLAGS="-DSPECASM_TARGET_NEXT_OPCODES"
fi

# Build the posix version of the tools with the correct flags.
# We'll need these for assembling and linking the .x file and
# binaries included in the release.

pushd $script_dir
make clean
make -j
popd

# Build fsize and copy into the release.

cp -r $script_dir/asm/fsize $release_dir
pushd $release_dir/fsize
$script_dir/saimport *.s
$script_dir/salink
popd
cp $release_dir/fsize/fsize $release_dir/specasm

# Build tst and copy into the release.

cp -r $script_dir/asm/tst $release_dir
pushd $release_dir/tst
$script_dir/saimport *.s *.ts
popd

mkdir $release_dir/specasm/lib
cp $release_dir/tst/*.x $release_dir/specasm/lib
cp $release_dir/tst/*.t $release_dir/specasm/lib
