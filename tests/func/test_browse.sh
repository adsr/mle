#!/usr/bin/env bash

if ! command -v tree &>/dev/null; then
    # skip if tree is not available
    skip='tree not in PATH'
    source 'test.sh'
    exit 0
fi

# make tmpdir and delete at exit
tmpdir=$(mktemp -d)
pushed=0
finish() { [ $pushed -eq 1 ] && popd &>/dev/null; rm -rf $tmpdir; }
trap finish EXIT

# make adir/bdir/hello
this_dir=$(pwd)
pushd $tmpdir &>/dev/null
pushed=1
mkdir -p adir/bdir
touch adir/bdir/hello

# ensure that we can browse into adir, then bdir, then select hello
macro='C-b C-f a d i r enter enter C-f b d i r enter enter C-f h e l l o enter enter'
declare -A expected
expected[hello]='^bview.[[:digit:]]+.buffer.path=./hello$'
source "$this_dir/test.sh"

popd &>/dev/null
pushed=0
