#!/usr/bin/env bash

macro='a M-h M-p M-c C-c b'
declare -A expected
expected[close_root_prompted]='^ab$'
source 'test.sh'
