#!/usr/bin/env bash

# cmd_move_(left|right|up|down)
macro='h e l l o enter w o r l d left up right down'
declare -A expected
expected[dpad_cursor_line]='^bview.0.cursor.0.mark.line_index=1$'
expected[dpad_cursor_col ]='^bview.0.cursor.0.mark.col=5$'
source 'test.sh'

# cmd_move_(bol)
macro='h e l l o enter w o r l d C-a'
declare -A expected
expected[bol_cursor_line]='^bview.0.cursor.0.mark.line_index=1$'
expected[bol_cursor_col ]='^bview.0.cursor.0.mark.col=0$'
source 'test.sh'

# cmd_move_(eol)
macro='h e l l o enter w o r l d M-\ C-e'
declare -A expected
expected[eol_cursor_line]='^bview.0.cursor.0.mark.line_index=0$'
expected[eol_cursor_col ]='^bview.0.cursor.0.mark.col=5$'
source 'test.sh'

# cmd_move_(beginning,end)
macro='h e l l o enter w o r l d M-\ M-/'
declare -A expected
expected[beg_end_cursor_line]='^bview.0.cursor.0.mark.line_index=1$'
expected[beg_end_cursor_col ]='^bview.0.cursor.0.mark.col=5$'
source 'test.sh'

# cmd_move_(to_line)
macro='0 enter 1 enter 2 enter 3 M-g 2 enter'
declare -A expected
expected[to_line_cursor_col]='^bview.0.cursor.0.mark.line_index=1$'
source 'test.sh'

# cmd_move_(to_offset)
macro='a a enter b b enter c c enter M-G 5 enter'
declare -A expected
expected[to_offset_cursor_line]='^bview.0.cursor.0.mark.line_index=1$'
expected[to_offset_cursor_col ]='^bview.0.cursor.0.mark.col=2$'
source 'test.sh'

# cmd_move_(to_relative)
macro='0 enter 1 enter 2 enter 3 enter M-y 2 u M-y 1 d'
declare -A expected
expected[relative_cursor_col]='^bview.0.cursor.0.mark.line_index=3$'
source 'test.sh'

# cmd_move_(until_forward|until_back)
macro="b a n a n a M-; b M-' a"
declare -A expected
expected[until_cursor_col]='^bview.0.cursor.0.mark.col=1$'
source 'test.sh'

# cmd_move_until_forward (nudge)
macro="a a C-a M-' a"
declare -A expected
expected[nudge_cursor_col]='^bview.0.cursor.0.mark.col=1$'
source 'test.sh'

# cmd_move_(word_forward|word_back)
macro='a p p l e space b a n a n a space c r a n M-b M-b M-f'
declare -A expected
expected[word_cursor_col]='^bview.0.cursor.0.mark.col=12$'
source 'test.sh'

# cmd_move_(bracket_forward|bracket_back)
macro='t e s t space { space { space { M-left M-left M-left M-right'
declare -A expected
expected[bracket_cursor_col]='^bview.0.cursor.0.mark.col=7$'
source 'test.sh'

# cmd_move_bracket_toggle
macro='t e s t 1 { enter t e s t 2 [ enter ] enter } M-= a M-= b'
declare -A expected
expected[bracket_toggle_line]='^bview.0.cursor.0.mark.line_index=2$'
expected[bracket_toggle_col ]='^bview.0.cursor.0.mark.col=1$'
expected[bracket_toggle_a   ]='^test2a\[$'
expected[bracket_toggle_b   ]='^b\]$'
source 'test.sh'

# cmd_jump
macro='a n t space b a t space c a t space d o g M-j a c'
declare -A expected
expected[jump_cursor_col]='^bview.0.cursor.0.mark.col=8$'
source 'test.sh'

# cmd_(drop|goto)_lettered_mark
macro='a enter b enter c M-m M-\ a M-m c up b M-z z M-m c M-z z b M-m c'
declare -A expected
expected[lettmark_a]='^aa$'
expected[lettmark_b]='^bbb$'
expected[lettmark_c]='^cccc$'
source 'test.sh'
