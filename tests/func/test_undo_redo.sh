#!/usr/bin/env bash

macro='a p p l e C-z C-z'
declare -A expected
expected[undo_data]='^app$'
source 'test.sh'

macro='a p p l e C-z C-z C-y'
declare -A expected
expected[redo_data]='^appl$'
source 'test.sh'
