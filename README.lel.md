# lel command reference

Motions

    /re/        next regex
    ?re?        prev regex
    'str'       next string
    "str"       prev string
    r/re/       next regex (/ is any delim)
    R/re/       prev regex
    f/str/      next string (/ is any delim)
    F/str/      prev string
    ta          next char (a is any char)
    Ta          prev char
    w           next word outer
    W           prev word outer
    n           next word inner
    N           next word inner
    l0          line absolute
    l-2         line relative
    #5          column absolute
    #+1         column relative
    ^           beginning of line
    $           end of line
    g           beginning of file
    G           end of file
    h           beginning of sel
    H           end of sel
    ma          mark (a is [a-z])
    ~           origin mark

Actions

    a/str/      insert string before cursor (/ is any delim)
    J           insert newline before cursor
    c/str/      change sel to string
    i/str/      insert string after cursor
    d           delete sel
    s/re/x/     regex replace sel
    k           cut sel
    y           copy sel
    Y           copy append sel
    v           paste
    |`cmd`      pipe sel to shell cmd (` is any delim)

Cursors

    D           drop cursor anchor
    U           lift cursor anchor
    z           drop sleeping cursor
    Z           awake all cursors
    .           collapse cursors
    O           swap cursor mark with anchor
    Ma          drop mark (a is [a-z])
    !           move user cursor to lel cursor

Registers

    <a          prepend sel to register (a is [a-z])
    >a          append sel to register
    =a          set register to sel
    _a          clear register
    Aa          write out register (before cursor)
    Ia          write out register (after cursor)
    Sa/re/str/  like A, but regex replace output

Loops, conditionals, etc

    L cmd       foreach line, do cmd
    x/re/cmd    foreach regex match, do cmd (/ is any delim)
    X/re/cmd    ditto, with dropped anchor
    q/re/cmd    if re matches sel, do cmd
    Q/re/cmd    if re does not match sel, do cmd
    { c1 c2 }   group commands
    9cmd        repeat command (9 is any number)

Examples

    Delete next 4 lines
        4d
    Find first occurrence of 'int'
        g 'int' !
    Find last occurrence of 'int'
        G "int" !
    Remove all tabs from file
        L s/\t//
    Multi-cursor select all matches of 'var'
        X/var/z Z
    Hard wrap file at column 79
        g D G |`fold -sw79`
    Comment out lines containing 'todo'
        L q/todo/ { ^ a|//| }
    Copy func protos here, replacing bracket with semi-colon
        x/^static.*{/>a ~ Sa/ {/;/ _a
