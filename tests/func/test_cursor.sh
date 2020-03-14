#!/bin/bash

macro='b e g i n space { enter l i n e 1 enter l i n e 2 enter } enter C-f l i n e 1 enter C-2 d'
declare -A expected
expected[sel_bracket_mark_line]='.mark.line_index=0$'
expected[sel_bracket_mark_col]='.mark.col=7$'
expected[sel_bracket_anchor_line]='.anchor.line_index=3$'
expected[sel_bracket_anchor_col]='.anchor.col=0$'
source 'test.sh'

macro='a b c enter l i n e 2 enter l i n e 3 C-2 q'
declare -A expected
expected[sel_all_mark_line]='.mark.line_index=2$'
expected[sel_all_mark_col]='.mark.col=5$'
expected[sel_all_anchor_line]='.anchor.line_index=0$'
expected[sel_all_anchor_col]='.anchor.col=0$'
source 'test.sh'
