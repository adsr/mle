#!/usr/bin/env bash

on_exit() { [ -n "$lua_script" ] && rm -f $lua_script; }
trap on_exit EXIT
lua_script=$(mktemp 'mle.test_lua.XXXXXXXXXX')
extra_opts="-x $lua_script -K lua_kmap,,1 -k cmd_lua_test,F11, -k cmd_quit_without_saving,F12, -n lua_kmap"

# mle.mark_insert_before
macro='F11'
cat >$lua_script <<"EOD"
mle.editor_register_cmd("cmd_lua_test", function (ctx)
    mle.mark_insert_before(ctx["mark"], "hello from lua\n", 15)
end)
EOD
declare -A expected
expected[simple_data]='^hello from lua$'
source 'test.sh'


# mle.editor_open_bview
macro='F11'
cat >$lua_script <<"EOD"
mle.editor_register_cmd("cmd_lua_test", function (ctx)
    print "hi1"
    mle.editor_open_bview(ctx["editor"], nil, 0, nil, 0, 1, 0, 0, nil)
    print "hi2"
end)
EOD
declare -A expected
expected[open_data1      ]='^hi1$'
expected[open_data2      ]='^hi2$'
expected[open_bview_count]='^bview_count=2$'
source 'test.sh'

# mle.editor_prompt
macro='F11 t e s t enter . . . F11 C-c'
cat >$lua_script <<"EOD"
mle.editor_register_cmd("cmd_lua_test", function (ctx)
    rv = mle.editor_prompt(ctx["editor"], "input?")
    if rv then
        str = "hello " .. rv .. " from lua"
    else
        str = "you hit ctrl-c"
    end
    mle.mark_insert_before(ctx["mark"], str, string.len(str))
end)
EOD
declare -A expected
expected[prompt_data]='^hello test from lua...you hit ctrl-c$'
source 'test.sh'

# mle.editor_register_observer
macro='F11'
cat >$lua_script <<"EOD"
mle.editor_register_cmd("cmd_lua_test", function (ctx)
    print "ell"
end)
mle.editor_register_observer("cmd:cmd_lua_test:before", function (ctx)
    print "h"
end)
mle.editor_register_observer("cmd:cmd_lua_test:after", function (ctx)
    print "o"
end)
EOD
declare -A expected
expected[observer_data]='^hello$'
source 'test.sh'

# mle.editor_register_observer
macro='F11 h i enter M-e s e q space 1 space 5 | p a s t e space - s d , space - enter backspace backspace'
cat >$lua_script <<"EOD"
mark = nil
in_callback = false
mle.editor_register_cmd("cmd_lua_test", function (ctx)
    mark = ctx["mark"]
    bview = mle.editor_open_bview(ctx["editor"], nil, 0, nil, 0, 1, 0, 0, nil)
    mle.editor_set_active(ctx["editor"], bview["optret_bview"])
end)
mle.editor_register_observer("buffer:baction", function (baction)
    if in_callback or not mark then return end
    in_callback = true
    str = "buffer=" .. baction["buffer"] .. " byte_delta=" .. baction["byte_delta"] .. "\n"
    mle.mark_insert_before(mark, str, string.len(str))
    in_callback = false
end)
EOD
declare -A expected
expected[observer_data1  ]='^hi$'
expected[observer_data2  ]='^1,2,3,4,$'
expected[observer_output1]='^buffer=\S+ byte_delta=1$'  # typing
expected[observer_output1]='^buffer=\S+ byte_delta=10$' # output from `seq 1 5 | paste -sd,`
expected[observer_output1]='^buffer=\S+ byte_delta=-1$' # backspacing
source 'test.sh'
