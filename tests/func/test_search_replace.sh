#!/usr/bin/env bash

# cmd_search
macro='h e l l o enter w o r l d C-f h e l l o enter'
declare -A expected
expected[search_cursor_line]='^bview.0.cursor.0.mark.line_index=0$'
expected[search_cursor_col ]='^bview.0.cursor.0.mark.col=0$'
source 'test.sh'

# cmd_search_next
macro='h e l l o enter h e l l o C-f h e l l o enter C-g'
declare -A expected
expected[next_cursor_line]='^bview.0.cursor.0.mark.line_index=1$'
expected[next_cursor_col ]='^bview.0.cursor.0.mark.col=0$'
source 'test.sh'

# cmd_search_next (wrap)
macro='h e l l o enter h e l l o C-f h e l l o enter C-g C-g'
declare -A expected
expected[next_wrap_cursor_line]='^bview.0.cursor.0.mark.line_index=0$'
expected[next_wrap_cursor_col ]='^bview.0.cursor.0.mark.col=0$'
source 'test.sh'

# cmd_find_word
macro='h e l l o enter h e l l o left C-v'
declare -A expected
expected[find_cursor_line]='^bview.0.cursor.0.mark.line_index=0$'
expected[find_cursor_col ]='^bview.0.cursor.0.mark.col=0$'
source 'test.sh'

# cmd_find_word (wrap)
macro='h e l l o enter h e l l o left C-v C-v'
declare -A expected
expected[find_wrap_cursor_line]='^bview.0.cursor.0.mark.line_index=1$'
expected[find_wrap_cursor_col ]='^bview.0.cursor.0.mark.col=0$'
source 'test.sh'

# cmd_isearch
macro='a c t o r enter a p p l e enter a p p e t i t e enter a z u r e M-\ C-r a p p e enter'
declare -A expected
expected[isearch_cursor_line]='^bview.0.cursor.0.mark.line_index=2$'
expected[iesarch_cursor_col ]='^bview.0.cursor.0.mark.col=0$'
source 'test.sh'

# cmd_replace 1
macro='a 1 space b 2 space c 3 space d 4 C-t \ d + enter x enter y n a'
declare -A expected
expected[replace1_data]='^ax b2 cx dx$'
source 'test.sh'

# cmd_replace 2
macro='a 1 space b 2 space c 3 space d 4 C-t \ d + enter x enter y n C-c'
declare -A expected
expected[replace2_data]='^ax b2 c3 d4$'
source 'test.sh'

# cmd_replace 3
macro='a b c 1 2 3 C-t ( . . . ) ( . . . ) enter A $ 2 $ n B $ 1 $ n $ x 4 3 enter a'
declare -A expected
expected[replace3_data1]='^A123$'
expected[replace3_data2]='^Babc$'
expected[replace3_data3]='^C$'
source 'test.sh'

# cmd_search history
macro='h e l l o enter h e l l o C-f h e l l o enter C-f up enter'
declare -A expected
expected[history_line]='^bview.0.cursor.0.mark.line_index=1$'
expected[history_col ]='^bview.0.cursor.0.mark.col=0$'
source 'test.sh'
