#!/bin/bash

on_exit() { [ -n "$wren_script" ] && rm -f $wren_script; }
trap on_exit EXIT
wren_script=$(mktemp 'mle.test_wren.XXXXXXXXXX')
extra_opts="-x $wren_script -K kmap_wren,,1 -k cmd_wren,F11, -n kmap_wren"
macro='F11'

# mle.mark_insert_before
cat >$wren_script <<"EOD"
mle.editor_register_cmd("cmd_wren", Fn.new {|ctx|
    mle.mark_insert_before(ctx["mark"], "hello from wren\n", 16)
})
EOD
declare -A expected
expected[simple_data]='^hello from wren$'
source 'test.sh'

# mle.editor_open_bview
cat >$wren_script <<"EOD"
mle.editor_register_cmd("cmd_wren", Fn.new {|ctx|
    System.print("hi1")
    mle.editor_open_bview(ctx["editor"], null, 0, null, 0, 1, 0, 0, null)
    System.print("hi2")
})
EOD
declare -A expected
expected[open_data1      ]='^hi1$'
expected[open_data2      ]='^hi2$'
expected[open_bview_count]='^bview_count=2$'
source 'test.sh'
