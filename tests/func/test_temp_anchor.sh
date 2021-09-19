#!/usr/bin/env bash

macro='o n e space t w o CS-left C-k o k'
declare -A expected
expected[temp_anchor_cut]='^one ok$'
source 'test.sh'

macro='o n e space t w o CS-left x C-k o k'
declare -A expected
expected[temp_anchor_cancel_cut]='^ok$'
source 'test.sh'
