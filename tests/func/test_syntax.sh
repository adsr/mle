#!/usr/bin/env bash

tmpf=test_syntax_file
extra_opts="-S syn_test,$tmpf,4,0 -s foo,,258,3 $tmpf"
macro='M-c f o o'
declare -A expected
expected[style_fg]='\bfg=258\b'
expected[style_bg]='\bbg=3\b'
source 'test.sh'
