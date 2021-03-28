#!/usr/bin/env bash

# make tmpf and delete at exit
tmpf=$(mktemp)
finish() { rm -f $tmpf; }
trap finish EXIT

# write >MLE_BRACKET_PAIR_MAX_SEARCH chars to tmpf
echo '('    >>$tmpf
seq 1 10000 >>$tmpf
echo ')'    >>$tmpf

# ensure cursor.sel_rule closed
macro='M-c right C-2 d'
extra_opts="$tmpf"
declare -A expected
expected[srule_should_be_closed]='^bview.0.cursor.0.sel_rule=n$'
source 'test.sh'
