#!/usr/bin/env bash

MLE_ORIG=$MLE
export MLE="timeout -s KILL 2 $MLE_ORIG"

macro='a C-z'
extra_opts='-u1' # coarse undo
declare -A expected
expected[no_inf_loop]='.'
source 'test.sh'

tmpf=$(mktemp)
finish() { rm -f $tmpf; }
trap finish EXIT
cat >$tmpf <<'EOD'
func apple() {
    // banana (ensure coarse undo works properly on very first user action)
} // carrot
EOD
byte_count=$(cat $tmpf | wc -c)
macro='C-e enter C-z'
extra_opts="-u1 -i1 $tmpf" # coarse undo, auto indent
declare -A expected
expected[coarse_undo_1st_1]="apple"
expected[coarse_undo_1st_2]="banana"
expected[coarse_undo_1st_3]="carrot"
expected[coarse_undo_1st_c]="^bview.0.buffer.byte_count=$byte_count\$"
source 'test.sh'

export MLE="$MLE_ORIG"
