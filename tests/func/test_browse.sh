#!/usr/bin/env bash

# skip if tree is not available
if ! command -v tree &>/dev/null; then
    skip='tree not in PATH'
    source 'test.sh'
    exit 0
elif ! tree --help | grep -q charset; then
    skip='tree version looks incorrect'
    source 'test.sh'
    exit 0
fi

# make tmpdir and delete at exit
this_dir=$(pwd)
tmpdir=$(mktemp -d)
cd $tmpdir
finish() { cd $this_dir; rm -rf $tmpdir; }
trap finish EXIT

# setup tmpdir
mkdir -p adir/bdir
touch adir/bdir/hi

# ensure that we can browse into adir, bdir, then select hi
# ensure path is relative to where we started
macro='C-b C-f a d i r enter enter C-f b d i r enter enter C-f h i enter enter'
declare -A expected
expected[hi]='^bview.[[:digit:]]+.buffer.path=adir/bdir/hi$'
source "$this_dir/test.sh"
