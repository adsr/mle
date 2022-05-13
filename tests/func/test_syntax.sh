#!/usr/bin/env bash

tmpf=test_syntax_file
extra_opts=(-S syn_test,$tmpf,4,0 -s foo,,258,3 $tmpf)
macro='M-c f o o'
declare -A expected
expected[style_fg]='\bfg=258\b'
expected[style_bg]='\bbg=3\b'
source 'test.sh'

tmpf=test_syntax_file
extra_opts=(-S syn_test,$tmpf,4,0 -s foo,,1,2 $tmpf)
macro='M-c { enter f o o ; enter } enter M-\ M-a M-/ M-. M-a'
declare -A expected
expected[outdent_style]='<ch=102 fg=1 bg=2><ch=111 fg=1 bg=2><ch=111 fg=1 bg=2><ch=59 fg=0 bg=0>'
source 'test.sh'
