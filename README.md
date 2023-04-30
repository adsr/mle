# mle

mle is a small, flexible, terminal-based text editor written in C.

Runs on Linux, Windows (Cygwin or WSL), FreeBSD, macOS, and more.

[![Build Status](https://travis-ci.org/adsr/mle.svg?branch=master)](https://travis-ci.org/adsr/mle)

[![Packaging status](https://repology.org/badge/vertical-allrepos/mle.svg)](https://repology.org/project/mle/versions)

### Demos

[![asciicast](https://i.imgur.com/PZocaOT.png)](https://asciinema.org/a/162536)

* [Emacs-style jump](https://i.imgur.com/atS11HX.gif)
* [Large file benchmark](http://i.imgur.com/VGGMmGg.gif)
* [Older demos](http://imgur.com/a/ZBmmQ)

### Aims

* Keep codebase small and hackable
* Minimize build-time and run-time dependencies
* Make extensible and configurable
* Favor simplicity over portability
* Use shell commands to enhance functionality (e.g., grep, tree)

### Features

* Small codebase (~10k sloc)
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

    $ sudo apt install git build-essential libtool automake # or equivalent
    $
    $ git clone --recursive https://github.com/adsr/mle.git
    $ cd mle
    $ make mle_vendor=1

To build a completely static binary, try `make mle_vendor=1 mle_static=1`.

You can also run plain `make` to link against system libraries instead of
`vendor/`. Note this requires the following packages to be installed:

    uthash-dev
    liblua5.4-dev
    libpcre2-dev

To install to `/usr/local/bin`:

    $ make install

To install to a custom directory, supply `prefix`, e.g.:

    $ make install prefix=/usr # /usr/bin/mle

### Installing from a repo

mle may be available to install via your system's package manager.

    # apt install mle   # Ubuntu and Debian-based distros
    # dnf install mle   # CentOS, RHEL, Fedora-based distros
    # pkg install mle   # FreeBSD
    # yay -S mle        # Arch (via AUR)
    # snap install mle  # all major Linux distros
    # nix-env -i mle    # NixOS (via nixpkgs)
    # apk add mle       # Alpine
    # xbps-install mle  # Void
    # brew install mle  # macOS (Homebrew)
    # port install mle  # macOS (MacPorts)
    # setup-x86.exe -q -P mle # Cygwin

### Basic usage

    $ mle               # Open blank buffer
    $ mle one.c         # Edit one.c
    $ mle one.c:100     # Edit one.c at line 100
    $ mle one.c two.c   # Edit one.c and two.c
    $ mle -h            # Show command line help

The default key bindings are intuitive. Input text as normal, use directional
keys to move around, use `Ctrl-S` to save, `Ctrl-O` to open, `Ctrl-X` to exit.

Press `F2` for full help.

### Advanced usage: mlerc

mle is customized via command line options. Run `mle -h` to view all cli
options.

To set default options, make an rc file named `~/.mlerc` (or `/etc/mlerc`). The
contents of the rc file are any number of cli options separated by newlines.
Lines that begin with a semi-colon are interpretted as comments.

If `~/.mlerc` is executable, mle executes it and interprets the resulting stdout
as described above. For example, consider the following snippet from an
executable `~/.mlerc` bash(1) script:

    # Define 'test' kmap
    echo '-Ktest,,1'

    # M-q: replace grep with git grep if `.git` exists
    if [ -d ".git" ]; then
      echo '-kcmd_grep,M-q,git grep --color=never -P -i -I -n %s 2>/dev/null'
    fi

    # Set default kmap
    echo '-n test'

This overrides the built-in grep command with `git grep` if `.git` exists in
the current working directory.

### Shell command integration

The following programs will enable or enhance certain features of mle if they
exist in `PATH`.

* [bash](https://www.gnu.org/software/bash/) (tab completion)
* [fzf](https://github.com/junegunn/fzf) (fuzzy file search)
* [grep](https://www.gnu.org/software/grep/) (file grep)
* [less](https://www.gnu.org/software/less/) (less integration)
* [perl](https://www.perl.org/) (perl 1-liners)
* [readtags](https://github.com/universal-ctags/ctags) (ctags integration)
* [tree](http://mama.indstate.edu/users/ice/tree/) (file browsing)

Arbitrary shell commands can also be run via `cmd_shell` (M-e by default). If
any text is selected, it is sent to stdin of the command. Any resulting stdout
is inserted into the text buffer.

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

### Advanced usage: Scripting

mle is extensible via the [Lua](https://www.lua.org) programming language.
Scripts are loaded via the `-x` cli option. Commands registered by scripts can
be mapped to keys as normal via `-k`. See `uscript.lua` for a simple example.

There is also a `wren` branch with [Wren](http://wren.io) scripting support.
That work is on pause.

### Forks

* [eon](https://github.com/tomas/eon) - mouse support and Notepad-like
  selections
* [turbo-mle](https://github.com/magiblot/turbo-mle) - Turbo Vision port

### Acknowledgments

mle makes extensive use of the following libraries.

* [uthash](https://troydhanson.github.io/uthash) for hash maps and linked lists
* [termbox2](https://github.com/termbox/termbox2) for TUI
* [PCRE2](http://www.pcre.org/) for syntax highlighting and search
