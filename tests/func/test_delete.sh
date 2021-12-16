#!/usr/bin/env bash

macro='f o o enter b a r backspace backspace backspace backspace'
declare -A expected
expected[bs_line1]='^foo$'
expected[bs_byte_count]='^bview.0.buffer.byte_count=3$'
expected[bs_line_count]='^bview.0.buffer.line_count=1$'
source 'test.sh'
unset expected

macro='f o o b a r left left left delete delete delete'
declare -A expected
expected[del_line1]='^foo$'
expected[del_byte_count]='^bview.0.buffer.byte_count=3$'
expected[del_line_count]='^bview.0.buffer.line_count=1$'
source 'test.sh'

macro='f o o space b a r C-w'
declare -A expected
expected[delete_word_back_data]='^foo $'
source 'test.sh'

macro='f o o space b a r C-a M-d'
declare -A expected
expected[delete_word_forward_data]='^ bar$'
source 'test.sh'
