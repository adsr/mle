#!/bin/bash

on_exit() { [ -n "$wren_script" ] && rm -f $wren_script; }
trap on_exit EXIT
wren_script=$(mktemp 'mle.test_wren.XXXXXXXXXX')
extra_opts="-x $wren_script -K kmap_wren,,1 -k cmd_wren_test,F11, -n kmap_wren"

# mle.mark_insert_before
macro='F11'
cat >$wren_script <<"EOD"
mle.editor_register_cmd("cmd_wren_test", Fn.new {|ctx|
    mle.mark_insert_before(ctx["mark"], "hello from wren\n", 16)
})
EOD
declare -A expected
expected[simple_data]='^hello from wren$'
source 'test.sh'

# mle.editor_open_bview
macro='F11'
cat >$wren_script <<"EOD"
mle.editor_register_cmd("cmd_wren_test", Fn.new {|ctx|
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

# mle.editor_prompt
macro='F11 t e s t enter . . . F11 C-c'
cat >$wren_script <<"EOD"
mle.editor_register_cmd("cmd_wren_test", Fn.new {|ctx|
    var rv = mle.editor_prompt(ctx["editor"], "input?")
    var str = ""
    if (rv[1]) {
        str = "hello %(rv[1]) from wren"
    } else {
        str = "you hit ctrl-c"
    }
    mle.mark_insert_before(ctx["mark"], str, str.bytes.count)
})
EOD
declare -A expected
expected[prompt_data]='^hello test from wren...you hit ctrl-c$'
source 'test.sh'

# mle.editor_register_observer
macro='F11'
cat >$wren_script <<"EOD"
mle.editor_register_cmd("cmd_wren_test", Fn.new {|ctx|
    System.write("ell")
})
mle.editor_register_observer("cmd:cmd_wren_test:before", Fn.new {|ctx|
    System.write("h")
})
mle.editor_register_observer("cmd:cmd_wren_test:after", Fn.new {|ctx|
    System.write("o")
})
EOD
declare -A expected
expected[prompt_data]='^hello$'
source 'test.sh'

# mle.editor_register_observer
set -x
macro='F11'
cat >$wren_script <<"EOD"
var mark = null
var in_callback = false
mle.editor_register_cmd("cmd_wren_test", Fn.new {|ctx|
    if (!mark) {
        var bview = mle.bview_new(ctx["editor"], null, 0, null)
        var cursor_ret = mle.bview_add_cursor(bview, null, 0)
        var cursor = cursor_ret[1]
        mark = mle.cursor_get_mark(cursor)
    }
})
mle.editor_register_observer("buffer:baction", Fn.new {|baction|
    if (in_callback) {
        return
    }
    in_callback = true
    mle.mark_insert_before("byte_delta was %(baction["byte_delta"])\n")
})
EOD
declare -A expected
expected[prompt_data]='^hello$'
source 'test.sh' # TODO Wren not re-entrant :(
