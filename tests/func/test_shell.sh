#!/usr/bin/env bash

macro='M-e e c h o space h e l l o 4 2 enter'
declare -A expected
expected[shell_data]='^hello42$'
source 'test.sh'
