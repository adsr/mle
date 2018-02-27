// mle -x uscript.wren -K kmap_wren,,1 -k cmd_wren_hello,F12, -n kmap_wren

mle.editor_register_cmd("cmd_wren_hello", Fn.new {|ctx|
    var rv = mle.editor_prompt(ctx["editor"], "What is your name?", null)
    var str = "Hello %(rv[1]) from Wren"
    mle.mark_insert_before(ctx["mark"], str, str.bytes.count)
})
