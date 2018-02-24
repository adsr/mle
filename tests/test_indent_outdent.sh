#!/bin/bash

# cmd_indent
macro='h e l l o M-.'
declare -A expected
expected[indent_data]='^    hello$'
source 'test.sh'

# cmd_outdent
macro='space space space space h e l l o M-,'
declare -A expected
expected[outdent_data]='^hello$'
source 'test.sh'

# cmd_indent (sel)
macro='h e l l o enter w o r l d M-\ M-a M-/ M-.'
declare -A expected
expected[sel-indent_data1]='^    hello$'
expected[sel-indent_data2]='^    world$'
source 'test.sh'

# cmd_outdent (sel)
macro='space space space space h e l l o enter space space space space w o r l d M-\ M-a M-/ M-,'
declare -A expected
expected[sel-indent_data1]='^hello$'
expected[sel-indent_data2]='^world$'
source 'test.sh'
