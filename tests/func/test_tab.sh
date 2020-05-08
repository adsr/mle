#!/usr/bin/env bash

extra_opts='-t 10 -y syn_generic'
macro='tab h e l l o'
declare -A expected
expected[tab_line2]='^          hello$'
source 'test.sh'

extra_opts='-t 10'
macro='tab h e l l o'
declare -A expected
expected[tab_line1]='^          hello$'
source 'test.sh'
