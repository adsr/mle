#!/usr/bin/env bash

macro="b a n a n a home M-' a F5 F5"
declare -A expected
expected[cursor]='^bview.0.cursor.0.mark.col=5$'
source 'test.sh'
