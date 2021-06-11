#!/usr/bin/env bash

macro=$(echo -en "f \xc3\xb6 r e t a g C-a M-f !")
extra_opts='-o (*UTF8)(*UCP)((?<=\w)\W|$)'
declare -A expected
expected[wide_word_forward_content]='^f.+retag!$'
source 'test.sh'
