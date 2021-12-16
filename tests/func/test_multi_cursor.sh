#!/usr/bin/env bash

macro='1 0 space 2 0 space 3 0 C-r \ d + C-/ y e s C-/ /'
declare -A expected
expected[isearch_data        ]='^yes10 yes20 yes30$'
expected[isearch_cursor_count]='^bview.0.cursor_count=1$'
source 'test.sh'

macro='h i space C-/ . C-a C-/ a X X'
declare -A expected
expected[drop-wake_data        ]='^XXhi XX$'
expected[drop-wake_cursor_count]='^bview.0.cursor_count=2$'
source 'test.sh'

macro="o n e enter t w o enter t h r e e M-a M-\ C-/ ' C-e space a n d"
declare -A expected
expected[column_data1       ]='^one and$'
expected[column_data2       ]='^two and$'
expected[column_data3       ]='^three and$'
expected[column_cursor_count]='^bview.0.cursor_count=3$'
source 'test.sh'

macro="a enter space b enter space space c enter C-r \ S + C-/ C-/ l"
declare -A expected
expected[align_line_1]='^  a$'
expected[align_line_2]='^  b$'
expected[align_line_3]='^  c$'
source 'test.sh'
