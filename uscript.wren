// mle -x uscript.wren -K kmap_wren,,1 -k wren_hello,F12, -n kmap_wren

mle.editor_register_cmd("wren_hello", Fn.new {|mark|
    mle.mark_insert_before(mark, "hi from Wren\n", 13)
    System.print("hi from System.print")
})
