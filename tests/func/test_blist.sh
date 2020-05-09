#!/usr/bin/env bash

extra_opts='file42 file43'
macro='C-\ C-f f i l e 4 3 enter enter M-c'
declare -A expected
expected[bview_count   ]='^bview_count=2'
expected[bview_file42  ]='^bview.[[:digit:]]+.buffer.path=file42$'
expected[bview_headless]='^bview.[[:digit:]]+.buffer.path=$'
source "test.sh"
