#!/bin/bash

macro='b e g i n space { enter l i n e 1 enter l i n e 2 enter } enter C-f l i n e 1 enter C-2 d'
declare -A expected
expected[cursor_mark_line]='.mark.line_index=0$'
expected[cursor_mark_col]='.mark.col=7$'
expected[cursor_anchor_line]='.anchor.line_index=3$'
expected[cursor_anchor_col]='.anchor.col=0$'
source 'test.sh'
