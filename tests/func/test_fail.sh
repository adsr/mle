#!/usr/bin/env bash

macro='f a i l'
declare -A expected
expected[fail]='^not here$'
source 'test.sh'
