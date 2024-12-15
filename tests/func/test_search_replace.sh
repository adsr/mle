#!/usr/bin/env bash

# cmd_search
macro='h e l l o enter w o r l d C-f h e l l o enter'
declare -A expected
expected[search_cursor_line]='^bview.0.cursor.0.mark.line_index=0$'
expected[search_cursor_col ]='^bview.0.cursor.0.mark.col=0$'
source 'test.sh'

# cmd_rsearch
macro='h i enter h i enter h i enter CM-f h i enter'
declare -A expected
expected[rsearch_cursor_line]='^bview.0.cursor.0.mark.line_index=2$'
expected[rsearch_cursor_col ]='^bview.0.cursor.0.mark.col=0$'
source 'test.sh'

# cmd_search_next
macro='h i enter h i enter h i enter C-f h i enter C-g'
declare -A expected
expected[next_cursor_line]='^bview.0.cursor.0.mark.line_index=1$'
expected[next_cursor_col ]='^bview.0.cursor.0.mark.col=0$'
source 'test.sh'

# cmd_search_prev (wrap)
macro='h i enter h i enter h i enter C-f h i enter CM-g'
declare -A expected
expected[prev_wrap_cursor_line]='^bview.0.cursor.0.mark.line_index=2$'
expected[prev_wrap_cursor_col ]='^bview.0.cursor.0.mark.col=0$'
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

# cmd_rfind_word
macro='h i enter h i enter h i C-a CM-v'
declare -A expected
expected[rfind_cursor_line]='^bview.0.cursor.0.mark.line_index=1$'
expected[rfind_cursor_col ]='^bview.0.cursor.0.mark.col=0$'
source 'test.sh'

# cmd_isearch 1
macro='a c t o r enter a p p l e enter a p p e t i t e enter a z u r e M-\ C-r a p p e enter'
declare -A expected
expected[isearch_cursor_line]='^bview.0.cursor.0.mark.line_index=2$'
expected[iesarch_cursor_col ]='^bview.0.cursor.0.mark.col=0$'
source 'test.sh'

# cmd_isearch 2
macro='i enter a enter b enter a enter b M-\ C-r a enter C-r b enter C-r C-n C-n'
declare -A expected
expected[isearch_cursor_line]='^bview.0.cursor.0.mark.line_index=3$'
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

# cmd_replace_all (all)
macro='1 a C-n 2 b C-n 3 c CM-t ( \ d ) ( \ w ) enter $ 2 $ 1 enter a'
declare -A expected
expected[replace_all_1_1]='^a1$'
expected[replace_all_1_2]='^b2$'
expected[replace_all_1_3]='^c3$'
source 'test.sh'

# cmd_replace_all (no, yes, cancel)
macro='1 a C-n 2 b C-n 3 c CM-t ( \ d ) ( \ w ) enter $ 2 $ 1 enter n y C-c'
declare -A expected
expected[replace_all_2_1]='^1a$'
expected[replace_all_2_2]='^b2$'
expected[replace_all_2_3]='^3c$'
source 'test.sh'

# cmd_replace_all (avoid infinite replacement)
macro='a enter C-t ^ enter enter a'
declare -A expected
expected[replace_no_inf_loop]='^a$'
source 'test.sh'
