# Changelog

## [1.7.2] - 2023-05-24

### changed

- switch from pcre to pcre2 ([`8d8673c`](https://github.com/adsr/mle/commit/8d8673c))
- rewrite syntax highlighting code ([`7231152`](https://github.com/adsr/mle/commit/7231152))
- upgrade to uthash 2.3.0 ([`fe53013`](https://github.com/adsr/mle/commit/fe53013))
- avoid `chdir` in `cmd_browse` ([`3253070`](https://github.com/adsr/mle/commit/3253070))

### added

- add block (overtype) mode ([`bffa40f`](https://github.com/adsr/mle/commit/bffa40f))
- add mouse support ([`37919fe`](https://github.com/adsr/mle/commit/37919fe), [`7791647`](https://github.com/adsr/mle/commit/7791647))
- add lettered-mark support ([`358eec1`](https://github.com/adsr/mle/commit/358eec1))
- add bview navigation commands ([`619ff20`](https://github.com/adsr/mle/commit/619ff20), [`cd34aed`](https://github.com/adsr/mle/commit/cd34aed))
- add commands for reverse searching ([`ee39003`](https://github.com/adsr/mle/commit/ee39003), [`9682069`](https://github.com/adsr/mle/commit/9682069), [`2cfbcce`](https://github.com/adsr/mle/commit/2cfbcce))
- add runtime debug features ([`b3c7f5c`](https://github.com/adsr/mle/commit/b3c7f5c), [`609231f`](https://github.com/adsr/mle/commit/609231f))

### fixed

- lint man page ([`8c1543e`](https://github.com/adsr/mle/commit/8c1543e)) (Raf Czlonka)

## [1.5.0] - 2022-05-27

### changed

- switch from termbox to termbox2 ([`f1c2293`](https://github.com/adsr/mle/commit/f1c2293))
- upgrade to lua 5.4 ([`fed1717`](https://github.com/adsr/mle/commit/fed1717))
- improve editor perf in large files ([`dd28398`](https://github.com/adsr/mle/commit/dd28398))

### added

- add support for ctrl, alt, shift modifiers ([`5f876bb`](https://github.com/adsr/mle/commit/5f876bb))
- add coarse undo ([`7e05653`](https://github.com/adsr/mle/commit/7e05653))
- add ability to sigstop and resume editor ([`a19ea77`](https://github.com/adsr/mle/commit/a19ea77))
- add `snapcraft.yaml` ([`ef2a38a`](https://github.com/adsr/mle/commit/ef2a38a))
- add `cmd_align_cursors` ([`891d2ed`](https://github.com/adsr/mle/commit/891d2ed))
- add `cmd_blist` for navigating buffers ([`9853838`](https://github.com/adsr/mle/commit/9853838))
- add `cmd_insert_newline_below` ([`b016957`](https://github.com/adsr/mle/commit/b016957))
- add `cmd_repeat` ([`eb37d13`](https://github.com/adsr/mle/commit/eb37d13))
- add `cmd_swap_anchor` ([`171f52b`](https://github.com/adsr/mle/commit/171f52b))
- add cursor_select_by "all" ([`2d11ddb`](https://github.com/adsr/mle/commit/2d11ddb))

### fixed

- fix kmap fallthru bug ([`66cd599`](https://github.com/adsr/mle/commit/66cd599))
- fix viewport use-after-free bugs ([`8fb367d`](https://github.com/adsr/mle/commit/8fb367d), [`2273cde`](https://github.com/adsr/mle/commit/2273cde))
- fix handling of tabs and wide chars in soft wrap mode ([`6b3a954`](https://github.com/adsr/mle/commit/6b3a954))
- fix mystery infinite loop ([`3a154dd`](https://github.com/adsr/mle/commit/3a154dd))
- fix leak in syntax highlighting code ([`1c49728`](https://github.com/adsr/mle/commit/1c49728)) (Francois Berder)
- permit zero-width chars ([`75136ae`](https://github.com/adsr/mle/commit/75136ae))
- fix macos compat bugs (Kevin Sjöberg)

## [1.4.3] - 2020-02-13

### added

- fallback to malloc if mmap fails ([`dd966bb`](https://github.com/adsr/mle/commit/dd966bb))

## [1.4.2] - 2019-09-25

### fixed

- fix keybind bug ([`31e89a6`](https://github.com/adsr/mle/commit/31e89a6))

## [1.4.1] - 2019-09-20

### fixed

- fix keybind bug ([`af0444b`](https://github.com/adsr/mle/commit/af0444b))

## [1.4.0] - 2019-09-14

### changed

- change to k&r pointer code style ([`02a5565`](https://github.com/adsr/mle/commit/02a5565))
- switch to semver

### added

- add `cmd_anchor_by` ([`5b42593`](https://github.com/adsr/mle/commit/5b42593))

## [1.3] - 2018-12-21

### changed

- reorganize source tree ([`6dba4fa`](https://github.com/adsr/mle/commit/6dba4fa))

### added

- add man page ([`2a25d51`](https://github.com/adsr/mle/commit/2a25d51))
- add `cmd_move_bracket_toggle` for bracket matching ([`21b2deb`](https://github.com/adsr/mle/commit/21b2deb))

## [1.2] - 2018-08-11

### added

- add lua scripting support ([`76f1c7a`](https://github.com/adsr/mle/commit/76f1c7a))
- add functional test suite ([`af56bea`](https://github.com/adsr/mle/commit/af56bea))
- add `cmd_jump` for two-letter cursor jumping ([`abd239c`](https://github.com/adsr/mle/commit/abd239c))
- add `cmd_perl` for executing perl ([`abd239c`](https://github.com/adsr/mle/commit/abd239c))
- add auto indent ([`9b84541`](https://github.com/adsr/mle/commit/9b84541))
- add ctag support ([`f708148`](https://github.com/adsr/mle/commit/f708148))
- add copy paste between buffers ([`5789313`](https://github.com/adsr/mle/commit/5789313))

### removed

- remove lel edit language ([`8fa4b7d`](https://github.com/adsr/mle/commit/8fa4b7d))
- remove stdio extensibility ([`a560633`](https://github.com/adsr/mle/commit/a560633))

### fixed

- fix getopt bug in bsd ([`4a49979`](https://github.com/adsr/mle/commit/4a49979)) (Leonardo Cecchi)

## [1.1] - 2017-03-29

_[initial release](https://lists.suckless.org/dev/1703/31240.html)_

[1.7.2]: https://github.com/adsr/mle/releases/tag/v1.7.2
[1.5.0]: https://github.com/adsr/mle/releases/tag/v1.5.0
[1.4.3]: https://github.com/adsr/mle/releases/tag/v1.4.3
[1.4.2]: https://github.com/adsr/mle/releases/tag/v1.4.2
[1.4.1]: https://github.com/adsr/mle/releases/tag/v1.4.1
[1.4.0]: https://github.com/adsr/mle/releases/tag/v1.4.0
[1.3]: https://github.com/adsr/mle/releases/tag/v1.3
[1.2]: https://github.com/adsr/mle/releases/tag/v1.2
[1.1]: https://github.com/adsr/mle/releases/tag/v1.1
