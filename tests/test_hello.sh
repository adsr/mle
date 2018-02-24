#!/bin/bash

macro='h e l l o'

declare -A expected
expected[output     ]='^hello$'
expected[bview_count]='^bview_count=1$'
expected[byte_count ]='\.byte_count=5$'
expected[line_count ]='\.line_count=1$'

source 'test.sh'
