#!/usr/bin/env bash

extra_opts=(file42 file43)
macro='C-\ M-\ C-f f i l e 4 3 enter enter M-c'
declare -A expected
expected[bview_count   ]='^bview_count=2'
expected[bview_file42  ]='^bview.[[:digit:]]+.buffer.path=file42$'
expected[bview_headless]='^bview.[[:digit:]]+.buffer.path=$'
source "test.sh"

extra_opts=(apple banana carrot)
macro='M-2 M-3 M-0 M-c M-1 M-c'
declare -A expected
expected[carrot_count   ]='^bview_count=2'
expected[carrot_file    ]='^bview.[[:digit:]]+.buffer.path=carrot$'
expected[carrot_headless]='^bview.[[:digit:]]+.buffer.path=$'
source "test.sh"
