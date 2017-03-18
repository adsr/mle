# mle

mle is a small but powerful console text editor written in C.

[![Build Status](https://travis-ci.org/adsr/mle.svg?branch=master)](https://travis-ci.org/adsr/mle)

### Features

* Small codebase (~10k sloc)
* Only 1 out-of-repo dependency (pcre)
* Full UTF-8 support
* Syntax highlighting
* Stackable key maps (modes)
* Extensible via stdio
* Scriptable rc file
* Built-in text editing language (lel)
* Key macros
* Multiple splittable windows
* Regex search and replace
* Large file support
* Incremental search
* Linear undo and redo
* Multiple cursors
* Headless mode
* Movement via [less](https://www.gnu.org/software/less/)
* Fuzzy file search via [fzf](https://github.com/junegunn/fzf)
* File browsing via [tree](http://mama.indstate.edu/users/ice/tree/)
* File grep via [grep](https://www.gnu.org/software/grep/)

### Building

    $ git clone https://github.com/adsr/mle.git
    $ cd mle
    $ git submodule update --init --recursive
    $ sudo apt-get install libpcre3-dev # or yum install pcre-devel, etc
    $ make

You can run `make mle_static` instead to build a static binary.

### Demos

![mle demo](http://i.imgur.com/7xGs8fM.gif)

View more demos [here](http://imgur.com/a/ZBmmQ).

View large-file startup benchmark [here](http://i.imgur.com/VGGMmGg.gif).

### mlerc

mle is customized via an rc file, `~/.mlerc` (or `/etc/mlerc`). The contents of
the rc file are any number of cli options separated by newlines. (Run `mle -h`
to view all cli options.) Lines that begin with a semi-colon are comments.

Alternatively, if `~/.mlerc` is executable, mle executes it and interprets the
resulting stdout as described above.

For example, my rc file is an executable php script, and includes the following
snippet, which overrides `grep` with `git grep` if `.git` exists in the current
working directory.

    <?php if (file_exists('.git')): ?>
    -kcmd_grep,M-q,git grep --color=never -P -i -I -n %s 2>/dev/null
    <?php endif; ?>

### Scripting

mle is extensible via any program capable of standard I/O. A simple
line-based request/response protocol enables user scripts to register commands
and invoke internal editor functions in order to perform complex editing tasks.
All messages are URL-encoded and end with a newline.

Example exchange between a user script and mle:

    usx -> mle    method=editor_register_cmd&params%5B%5D=hello&id=57cc98bb168ae
    mle -> usx    result%5Brc%5D=0&id=57cc98bb168ae
    ...
    mle -> usx    method=hello&params%5Bmark%5D=76d6e0&params%5Bstatic_param%5D=&id=0x76d3a0-0
    usx -> mle    method=mark_insert_before&params%5B%5D=76d6e0&params%5B%5D=hello%3F&params%5B%5D=5&id=57cc98bb6ab3d
    mle -> usx    result%5Brc%5D=0&id=57cc98bb6ab3d
    usx -> mle    result%5Brc%5D=0&error=&id=0x76d3a0-0

In the example above, the user script registers a command called `hello` at
startup, and mle replies with success. Later, the end-user invokes the `hello`
command, so mle sends a request to the user script. The user script receives the
request and sends a sub-request invoking `mark_insert_before`, to which mle
replies with success. Finally the user script returns overall success for the
`hello` command.

Currently, mle only accepts requests from user scripts while a request to the
user script itself is pending. (In other words, mle enforces an "only do stuff
if I ask you to" policy.) The exception to this is `editor_register_cmd` which
can be invoked by user scripts at startup time.

For end-users, user scripts are loaded via the `-x` cli option. Commands
registered by user scripts can be mapped to keys as normal via `-k`.

### lel: edit language

mle comes with a built-in text editing language called lel, inspired by the
[sam command language](http://doc.cat-v.org/bell_labs/sam_lang_tutorial/). lel
commands can be entered manually at a prompt, or bound to any key via
`-kcmd_lel,<key>,<lelcmd>`.

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

### Headless mode

mle provides support for non-interactive editing which may be useful for using
the editor as a regular command line tool. In headless mode, mle reads stdin
into a buffer, applies a startup macro if specified, and then writes the buffer
contents to stdout. For example:

    $ echo -n hello | mle -M 'test C-e space w o r l d enter' -p test
    hello world

If stdin is a pipe, mle goes into headless mode automatically. Headless mode can
be explicitly enabled or disabled with the `-H` option.

### Runtime dependencies (optional)

The following programs will enable or enhance certain features of mle if they
exist in `PATH`.

* [bash](https://www.gnu.org/software/bash/) (tab completion)
* [fzf](https://github.com/junegunn/fzf) (fuzzy file search)
* [grep](https://www.gnu.org/software/grep/) (file grep)
* [less](https://www.gnu.org/software/less/) (less integration)
* [tree](http://mama.indstate.edu/users/ice/tree/) (file browsing)

### Known bugs

* Multi-line style rules don't work properly when overlapped/staggered.
* There's a segfault lurking in the window split code. I can't reliably
  reproduce it.

### Fork

Also check out [eon](https://github.com/tomas/eon), a fork of mle with some cool
features.

### Acknowledgements

mle makes extensive use of the following libraries.

* [uthash](https://troydhanson.github.io/uthash) for hash maps and linked lists
* [termbox](https://github.com/nsf/termbox) for TUI
* [PCRE](http://www.pcre.org/) for syntax highlighting and search
