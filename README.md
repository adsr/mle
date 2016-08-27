# mle

mle is a small but powerful console text editor written in C.

[![Build Status](https://travis-ci.org/adsr/mle.svg?branch=master)](https://travis-ci.org/adsr/mle)

### Features

* Small codebase (~6k sloc)
* Only 1 out-of-repo dependency (pcre)
* Full UTF-8 support
* Syntax highlighting
* Stackable key maps (modes)
* Macros
* Multiple windows
* Window splitting
* Regex search and replace
* Incremental search
* Fuzzy file search via [fzf](https://github.com/junegunn/fzf)
* File browsing via [tree](http://mama.indstate.edu/users/ice/tree/)
* File grep
* Multiple cursors
* Scriptable rc file
* Mutate text via any shell command

### Building

    $ git clone https://github.com/adsr/mle.git
    $ cd mle
    $ git submodule update --init --recursive
    $ sudo apt-get install libpcre3-dev # or yum install pcre-devel, etc
    $ make

You can run `make mle_static` instead to build a static binary.

### mlerc

mle is customized via an rc file, `~/.mlerc` (or `/etc/mlerc`). The contents of
the rc file are any number of cli options separated by newlines. (Run `mle -h`
to view all cli options.) Lines that begin with a semi-colon are comments.

Alternatively, if `~/.mlerc` executable, mle executes it and interprets the
resulting stdout as described above.

For example, my rc file is an executable php script, and includes the following
snippet, which overrides `grep` with `git grep` if `.git` exists in the current
working directory.

    <?php if (file_exists('.git')): ?>
    -kcmd_grep,M-q,git grep --color=never -P -i -I -n %s 2>/dev/null
    <?php endif; ?>

### Demos

![mle demo](http://i.imgur.com/7xGs8fM.gif)

View more demos [here](http://imgur.com/a/ZBmmQ).

### Known bugs

* Multi-line style rules don't work properly when overlapped/staggered.
* There's a segfault lurking in the window split code. I can't reliably
  reproduce it.

### Acknowledgements

mle makes extensive use of the following libraries.

* [uthash](https://troydhanson.github.io/uthash) for hash maps and linked lists
* [termbox](https://github.com/nsf/termbox) for TUI
* [PCRE](http://www.pcre.org/) for syntax highlighting and search
