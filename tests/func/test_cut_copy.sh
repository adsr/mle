#!/usr/bin/env bash

macro='f o o C-k'
declare -A expected
expected[cut_data]='^$'
source 'test.sh'

macro='f o o C-k C-u'
declare -A expected
expected[uncut_data]='^foo$'
source 'test.sh'

macro='f o o M-k C-u'
declare -A expected
expected[copy_data]='^foofoo$'
source 'test.sh'

macro='f o o M-a left left C-k'
declare -A expected
expected[sel_cut_data]='^f$'
source 'test.sh'

macro='f o o M-a left left C-k C-u'
declare -A expected
expected[sel_uncut_data]='^foo$'
source 'test.sh'

macro='f o o M-a left left M-k C-u'
declare -A expected
expected[sel_copy_data]='^foooo$'
source 'test.sh'

macro='a b c { x y z } C-f x y z enter C-d d C-e C-u'
declare -A expected
expected[cut_by_bracket_data]='^abc\{\}xyz$'
source 'test.sh'

macro='a b c space x y z left C-d w C-a C-u'
declare -A expected
expected[cut_by_word_data]='^xyzabc $'
source 'test.sh'

macro='a b c space x y z C-d s C-a C-u'
declare -A expected
expected[cut_by_word_back_data]='^xyzabc $'
source 'test.sh'

macro='a b c space x y z C-a C-d f C-e C-u'
declare -A expected
expected[cut_by_word_forward_data]='^ xyzabc$'
source 'test.sh'

macro='a b c space x y z C-d a C-u C-u'
declare -A expected
expected[cut_by_bol_data]='^abc xyzabc xyz$'
source 'test.sh'

macro='a b c space x y z C-a C-d e C-u C-u'
declare -A expected
expected[cut_by_eol_data]='^abc xyzabc xyz$'
source 'test.sh'

macro='a space " q u o t e d " space s t r i n g C-f u o t e enter C-d c'
declare -A expected
expected[cut_by_eol_data]='^a "" string$'
source 'test.sh'

macro='o n e - a b c M-a left left left C-k C-n t w o - C-u'
declare -A expected
expected[global_cut_buffer_data]='^two-abc$'
source 'test.sh'

macro='a b c C-2 e'
declare -A expected
expected[mark]='^bview.0.cursor.0.mark.col=3$'
expected[anchor]='^bview.0.cursor.0.anchor.col=3$'
source 'test.sh'

macro='a b c enter d e f M-\ M-a C-f f enter C-/ ; right C-k'
declare -A expected
expected[swap_anchor]='^af$'
source 'test.sh'

macro='a C-n b S-left M-k C-u M-p S-left M-k M-n CM-u'
declare -A expected
expected[buf1_data]='^a$'
expected[buf2_data]='^bab$'
source 'test.sh'
