#!/usr/bin/env bash

macro='h e l l o enter w o r l d enter'
declare -A expected
expected[a_line1     ]='^hello$'
expected[a_line2     ]='^world$'
expected[a_byte_count]='^bview.0.buffer.byte_count=12$'
expected[a_line_count]='^bview.0.buffer.line_count=3$'
source 'test.sh'

macro='w o r l d M-i h e l l o'
declare -A expected
expected[b_line1     ]='^hello$'
expected[b_line2     ]='^world$'
expected[b_byte_count]='^bview.0.buffer.byte_count=11$'
expected[b_line_count]='^bview.0.buffer.line_count=2$'
source 'test.sh'

macro='h e l l o C-a M-u right M-u C-e M-u'
declare -A expected
expected[c_line1     ]='^hello$'
expected[c_lineb     ]='^$'
expected[c_line_count]='^bview.0.buffer.line_count=4$'
source 'test.sh'
