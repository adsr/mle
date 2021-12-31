-- mle -N -x uscript.lua -K lua_kmap,,1 -k cmd_lua_test,f11, -n lua_kmap

mle.editor_register_observer("buffer:save", function (bview)
    r = mle.util_shell_exec("ls", -1)
    print("ls " .. r["output"])
end)

mle.editor_register_cmd("cmd_lua_test", function (ctx)
    name = mle.editor_prompt("Enter your name")
    if name then
        print("hello <" .. name .. "> from lua")
    else
        print("you hit cancel")
    end
end)

mle.editor_register_observer("cmd:cmd_copy:before", function (ctx)
    local rv = mle.cursor_get_anchor(ctx["cursor"])
    local anchor = rv["ret_anchor"]

    rv = mle.mark_get_between_mark(ctx["mark"], anchor)
    rv = mle.util_escape_shell_arg(rv["ret_str"])

    mle.util_shell_exec("echo -n " .. rv["output"] .. " | xclip -sel c >/dev/null", 1)
end)
