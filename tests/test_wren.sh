#!/bin/bash

on_exit() { [ -n "$wren_script" ] && rm -f $wren_script; }
trap on_exit EXIT

wren_script=$(mktemp 'mle.test_wren.XXXXXXXXXX')
cat >$wren_script <<"EOD"
mle.editor_register_cmd("cmd_wren", Fn.new {|mark|
    mle.mark_insert_before(mark, "hello from wren\n", 16)
})
EOD

extra_opts="-x $wren_script -K kmap_wren,,1 -k cmd_wren,F12, -n kmap_wren"
macro='F12'
declare -A expected
expected[wren_data]='^hello from wren$'
source 'test.sh'
