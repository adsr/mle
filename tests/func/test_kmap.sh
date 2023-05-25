#!/usr/bin/env bash

# ensure binding in prev kmap works with fallthru enabled
extra_opts=(
    -K 'ka,,1' -k 'cmd_shell,a a,printf A' -k 'cmd_push_kmap,x,kb' -k 'cmd_quit_without_saving,f12,'
    -K 'kb,,1' -k 'cmd_shell,a b,printf B' -k 'cmd_pop_kmap,x,'
    -n 'ka'
)
macro='a a a b x a b a a x a z'
declare -A expected
expected[data]='^AabBAaz$'
source 'test.sh'

# ensure input trail resets on non-matching input w/ fallthru disabled
extra_opts=(
    -K 'ka,,0' -k 'cmd_shell,a,printf A' -k 'cmd_quit_without_saving,f12,'
    -n 'ka'
)
macro='b a'
declare -A expected
expected[data]='^A$'
source 'test.sh'
