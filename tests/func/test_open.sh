#!/usr/bin/env bash

macro=''
extra_opts=(test_file_w_suffix test_file)
declare -A expected
expected[path_1]='^bview.[[:digit:]]+.buffer.path=test_file_w_suffix$'
expected[path_2]='^bview.[[:digit:]]+.buffer.path=test_file$'
source 'test.sh'
