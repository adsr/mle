#!/usr/bin/env bash

macro="b a n a n a home M-' a f5 f5"
declare -A expected
expected[cursor]='^bview.0.cursor.0.mark.col=5$'
source 'test.sh'
