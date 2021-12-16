#!/usr/bin/env bash

macro='C-n'
declare -A expected
expected[new_bview_count]='^bview_count=2$'
source 'test.sh'

macro='C-n M-c'
declare -A expected
expected[close_bview_count]='^bview_count=1$'
source 'test.sh'

macro='a p p l e C-n b a n a n a'
declare -A expected
expected[new2_data1]='^apple$'
expected[new2_data2]='^banana$'
source 'test.sh'

macro='a p p l e C-n b a n a n a M-p r e d M-n y e l l o w'
declare -A expected
expected[next_prev_data1]='^applered$'
expected[next_prev_data2]='^bananayellow$'
source 'test.sh'
