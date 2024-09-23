#!/usr/bin/env bash

extra_opts=(-M 'printable a A S-a M-a M-A MS-a C-a CS-a CM-a CMS-a enter comma tab f1' )
macro='M-< p r i n t a b l e enter'
declare -A expected
expected[printed_macro]='^a A S-a M-a M-A MS-a C-a CS-a CM-a CMS-a C-enter comma C-tab f1 $'
source 'test.sh'
