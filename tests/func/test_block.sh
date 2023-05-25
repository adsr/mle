#!/usr/bin/env bash

macro='a b c enter d e f enter g h i enter M-\ M-a insert j k down C-k C-u'
declare -A expected
expected[acut_line1]='^  c$'
expected[acut_line2]='^  jk$'
expected[acut_line3]='^ghde$'
source 'test.sh'

macro='a b c enter d e f enter g h i enter M-\ M-a insert right right down M-k C-u'
declare -A expected
expected[acopy_line1]='^abc$'
expected[acopy_line2]='^deab$'
expected[acopy_line3]='^ghde$'
source 'test.sh'

macro='space a b enter enter space c d M-a insert M-; a M-k C-e C-u'
declare -A expected
expected[aspace_line1]='^ abab$'
expected[aspace_line2]='^ cdcd$'
source 'test.sh'

macro='a b c insert M-k C-u'
declare -A expected
expected[copy_line1]='^abc$'
expected[copy_line2]='^$'
expected[copy_count]='\.line_count=2$'
source 'test.sh'

macro='a b c insert C-k C-u'
declare -A expected
expected[cut_line1]='^   abc$'
expected[cut_line2]='^   $'
expected[cut_count]='\.line_count=2$'
source 'test.sh'

macro="a enter b enter c M-\ M-a insert M-/ C-/ ' x"
declare -A expected
expected[drop_line1]='^ax$'
expected[drop_line2]='^bx$'
expected[drop_line3]='^cx$'
expected[drop_count]='\.line_count=3$'
source 'test.sh'

macro="a a enter b b enter c c M-\ M-a insert M-/ C-/ ' M-a C-a C-k C-u"
declare -A expected
expected[adrop_line1]='^  aa$'
expected[adrop_line2]='^  bb$'
expected[adrop_line3]='^  cc$'
expected[adrop_line4]='^  $'
expected[adrop_count]='\.line_count=4$'
source 'test.sh'
