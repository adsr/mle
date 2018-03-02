// mle -x uscript.wren -K kmap_wren,,1 -k cmd_wren_hello,F12, -n kmap_wren

mle.editor_register_cmd("cmd_wren_hello", Fn.new {|ctx|
    var rv = mle.editor_prompt(ctx["editor"], "What is your name?")
    var str = "Hello %(rv[1]) from Wren\n"
    mle.mark_insert_before(ctx["mark"], str, str.bytes.count)
})

mle.editor_register_observer("cmd_wren_hello", 1, Fn.new {|ctx|
    System.print("pre cmd_wren_hello")
})
mle.editor_register_observer("cmd_wren_hello", 0, Fn.new {|ctx|
    System.print("post cmd_wren_hello")
})
