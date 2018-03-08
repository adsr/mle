# mle

mle is a small, flexible, terminal-based text editor written in C.

[![Build Status](https://travis-ci.org/adsr/mle.svg?branch=master)](https://travis-ci.org/adsr/mle)

### Demos

[![asciicast](https://i.imgur.com/PZocaOT.png)](https://asciinema.org/a/162536)

* [Emacs-style jump](https://i.imgur.com/atS11HX.gif)
* [Large file benchmark](http://i.imgur.com/VGGMmGg.gif)
* [Older demos](http://imgur.com/a/ZBmmQ)

### Aims

* Keep codebase small
* Minimize build-time and run-time dependencies
* Make extensible and configurable
* Favor simplicity over portability
* Use shell commands to enhance functionality (e.g., grep, tree)

### Features

* Small codebase (<10k sloc)
* Only 1 out-of-repo dependency (PCRE)
* Full UTF-8 support
* Syntax highlighting
* Stackable key maps (modes)
* Extensible via [Lua](https://www.lua.org)
* Scriptable rc file
* Key macros
* Multiple splittable windows
* Regex search and replace
* Large file support
* Incremental search
* Linear undo and redo
* Multiple cursors
* Auto indent
* Headless mode
* Navigation via [ctags](https://github.com/universal-ctags/ctags)
* Movement via [less](https://www.gnu.org/software/less/)
* Fuzzy file search via [fzf](https://github.com/junegunn/fzf)
* File browsing via [tree](http://mama.indstate.edu/users/ice/tree/)
* File grep via [grep](https://www.gnu.org/software/grep/)
* String manip via [perl](https://www.perl.org/)

### Building

    $ git clone --recursive https://github.com/adsr/mle.git
    $ cd mle
    $ sudo apt-get install libpcre3-dev # or `yum install pcre-devel`, `pacman -S pcre`, etc
    $ make

You can run `make mle_static` instead to build a static binary.

### Installing

To install to `/usr/local/bin`:

    $ make install

To install to a custom directory, supply `DESTDIR`, e.g.:

    $ DESTDIR=/usr/bin make install

### Basic usage

    $ mle             # Blank buffer
    $ mle one.c       # Open one.c
    $ mle one.c:100   # Open one.c at line 100
    $ mle one.c two.c # Open one.c and two.c
    $ mle -h          # Show command line help

Press F2 for help when inside the editor.

### Advanced usage: mlerc

mle is customized via command line options. Run `mle -h` to view all cli
options.

To set default options, make an rc file named `~/.mlerc` (or `/etc/mlerc`). The
contents of the rc file are any number of cli options separated by newlines.
Lines that begin with a semi-colon are interpretted as comments.

If `~/.mlerc` is executable, mle executes it and interprets the resulting stdout
as described above. For example, consider the following snippet from an
executable `~/.mlerc` PHP script:

    <?php if (file_exists('.git')): ?>
    -kcmd_grep,M-q,git grep --color=never -P -i -I -n %s 2>/dev/null
    <?php endif; ?>

This overrides the normal grep command with `git grep` if `.git` exists in the
current working directory.

### Advanced usage: Scripting

mle is extensible via the [Lua](https://www.lua.org) programming language.
Scripts are loaded via the `-x` cli option. Commands registered by scripts can
be mapped to keys as normal via `-k`. See `uscript.lua` for a simple example.

There is also a `wren` branch with [Wren](http://wren.io) scripting support.
That work is on pause.

### Advanced usage: Headless mode

mle provides support for non-interactive editing which may be useful for using
the editor as a regular command line tool. In headless mode, mle reads stdin
into a buffer, applies a startup macro if specified, and then writes the buffer
contents to stdout. For example:

    $ echo -n hello | mle -M 'test C-e space w o r l d enter' -p test
    hello world

If stdin is a pipe, mle goes into headless mode automatically. Headless mode can
be explicitly enabled or disabled with the `-H` option.

If stdin is a pipe and headless mode is disabled via `-H0`, mle reads stdin into
a new buffer and then runs as normal in interactive mode.

### Runtime dependencies (optional)

The following programs will enable or enhance certain features of mle if they
exist in `PATH`.

* [bash](https://www.gnu.org/software/bash/) (tab completion)
* [fzf](https://github.com/junegunn/fzf) (fuzzy file search)
* [grep](https://www.gnu.org/software/grep/) (file grep)
* [less](https://www.gnu.org/software/less/) (less integration)
* [perl](https://www.perl.org/) (perl 1-liners)
* [readtags](https://github.com/universal-ctags/ctags) (ctags integration)
* [tree](http://mama.indstate.edu/users/ice/tree/) (file browsing)

### Known bugs

* Multi-line style rules don't work properly when overlapped/staggered.

### Fork

Also check out [eon](https://github.com/tomas/eon), a fork of mle with some cool
features.

### Acknowledgments

mle makes extensive use of the following libraries.

* [uthash](https://troydhanson.github.io/uthash) for hash maps and linked lists
* [termbox](https://github.com/nsf/termbox) for TUI
* [PCRE](http://www.pcre.org/) for syntax highlighting and search
