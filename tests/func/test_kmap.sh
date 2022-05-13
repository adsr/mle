#!/usr/bin/env bash

extra_opts=(
    -K 'ka,,1' -k 'cmd_shell,a a,echo -n A' -k 'cmd_push_kmap,x,kb' -k 'cmd_quit_without_saving,f12,'
    -K 'kb,,1' -k 'cmd_shell,a b,echo -n B' -k 'cmd_pop_kmap,x,'
    -n 'ka'
)
macro='a a a b x a b a a x a z'
declare -A expected
expected[data]='^AabBAaz$'
source 'test.sh'
