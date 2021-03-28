#!/usr/bin/env bash

macro='a C-z'
extra_opts='-u1' # coarse undo
declare -A expected
expected[no_inf_loop]='.'
MLE_ORIG=$MLE
export MLE="timeout -s KILL 2 $MLE_ORIG"
source 'test.sh'
export MLE="$MLE_ORIG"
