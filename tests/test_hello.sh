#!/bin/bash

macro='h e l l o'

declare -A expected
expected[data      ]='^hello$'
expected[byte_count]='^bview.0.buffer.byte_count=5$'
expected[line_count]='^bview.0.buffer.line_count=1$'

source 'test.sh'
