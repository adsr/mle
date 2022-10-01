prefix?=/usr/local

mle_cflags:=-std=c99 -Wall -Wextra -pedantic -Wno-pointer-arith -Wno-unused-result -Wno-unused-parameter -g -O0 -D_GNU_SOURCE -DPCRE2_CODE_UNIT_WIDTH=8 -I. $(CFLAGS)
mle_ldflags:=$(LDFLAGS)
mle_pcre2_cflags:=$(shell pkg-config --cflags libpcre2-8)
mle_pcre2_libs:=$(shell pkg-config --libs libpcre2-8)
mle_lua_libs:=$(shell pkg-config --libs lua5.4)
mle_lua_cflags:=$(shell pkg-config --cflags lua5.4)
mle_uthash_cflags:=$(shell pkg-config --cflags uthash)
mle_objects:=$(patsubst %.c,%.o,$(wildcard *.c))
mle_objects_no_main:=$(filter-out main.o,$(mle_objects))
mle_func_tests:=$(wildcard tests/func/test_*.sh))
mle_unit_tests:=$(patsubst %.c,%,$(wildcard tests/unit/test_*.c))
mle_unit_test_objects:=$(patsubst %.c,%.o,$(wildcard tests/unit/test_*.c))
mle_unit_test_all:=tests/unit/test

mle_ldlibs:=$(mle_pcre2_libs) $(mle_lua_libs) -lm $(LDLIBS)
mle_cflags+=$(mle_pcre2_cflags) $(mle_lua_cflags) $(mle_uthash_cflags) $(mle_cflags)

all: mle

mle: $(mle_objects) termbox2.h
	$(CC) $(mle_cflags) $(mle_objects) $(mle_ldflags) $(mle_ldlibs) -o mle

$(mle_objects): %.o: %.c
	$(CC) -c $(mle_cflags) $< -o $@

$(mle_unit_test_objects): %.o: %.c
	$(CC) -DTEST_NAME=$(basename $(notdir $<)) -c $(mle_cflags) -rdynamic $< -o $@

$(mle_unit_test_all): $(mle_objects_no_main) $(mle_unit_test_objects) $(mle_unit_test_all).c
	$(CC) $(mle_cflags) -rdynamic $(mle_unit_test_all).c $(mle_objects_no_main) $(mle_unit_test_objects) $(mle_ldflags) $(mle_ldlibs) -ldl -o $@

$(mle_unit_tests): %: $(mle_unit_test_all)
	{ echo "#!/bin/sh"; echo "$(abspath $(mle_unit_test_all)) $(notdir $@)"; } >$@ && chmod +x $@

test: mle $(mle_unit_tests)
	./mle -v && export MLE=$$(pwd)/mle && $(MAKE) -C tests

sloc:
	find . -name '*.c' -or -name '*.h' | \
		grep -Pv '(termbox|test|ut|lua)' | \
		xargs -rn1 cat | \
		wc -l

install: mle
	install -v -d $(DESTDIR)$(prefix)/bin
	install -v -m 755 mle $(DESTDIR)$(prefix)/bin/mle

uscript:
	php uscript.inc.php >uscript.inc

clean_quick:
	rm -f mle $(mle_objects) $(mle_unit_test_objects) $(mle_unit_tests) $(mle_unit_test_all)

clean:
	rm -f mle $(mle_objects) $(mle_unit_test_objects) $(mle_unit_tests) $(mle_unit_test_all)
	$(MAKE) -C clean

.PHONY: all test sloc install uscript clean
