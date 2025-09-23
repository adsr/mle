prefix?=/usr/local

mle_cflags:=-std=c99 -Wall -Wextra -pedantic -Wno-pointer-arith -Wno-unused-result -Wno-unused-parameter -g -O0 -D_GNU_SOURCE -DPCRE2_CODE_UNIT_WIDTH=8 -I. $(CFLAGS) $(CPPFLAGS)
mle_ldflags:=$(LDFLAGS)
mle_dynamic_libs:=-lpcre2-8 -llua5.4
mle_static_libs:=vendor/pcre2/libpcre2-8.a vendor/lua/liblua5.4.a
mle_ldlibs:=-lm $(LDLIBS)
mle_objects:=$(patsubst %.c,%.o,$(filter-out %.inc.c,$(wildcard *.c)))
mle_objects_no_main:=$(filter-out main.o,$(mle_objects))
mle_func_tests:=$(wildcard tests/func/test_*.sh))
mle_unit_tests:=$(patsubst %.c,%,$(wildcard tests/unit/test_*.c))
mle_unit_test_objects:=$(patsubst %.c,%.o,$(wildcard tests/unit/test_*.c))
mle_unit_test_all:=tests/unit/test
mle_vendor_deps:=
mle_static_var:=
mle_git_sha:=$(shell test -d .git && command -v git >/dev/null && git rev-parse --short HEAD)

ifdef mle_static
  mle_static_var:=-static
endif

ifdef mle_vendor
  mle_ldlibs:=$(mle_static_libs) $(mle_ldlibs)
  mle_cflags:=-Ivendor/pcre2/src -Ivendor -Ivendor/uthash/src $(mle_cflags)
  mle_vendor_deps:=$(mle_static_libs)
else
  mle_ldlibs:=$(mle_dynamic_libs) $(mle_ldlibs)
endif

ifneq ($(mle_git_sha),)
  mle_cflags:=-DMLE_VERSION_APPEND=-$(mle_git_sha) $(mle_cflags)
endif

all: mle

mle: $(mle_vendor_deps) $(mle_objects) termbox2.h
	$(CC) $(mle_static_var) $(mle_cflags) $(mle_objects) $(mle_ldflags) $(mle_ldlibs) -o mle

$(mle_objects): %.o: %.c
	$(CC) -c $(mle_cflags) $< -o $@

$(mle_vendor_deps):
	$(MAKE) -C vendor

$(mle_unit_test_objects): %.o: %.c
	$(CC) -DTEST_NAME=$(basename $(notdir $<)) -c $(mle_cflags) $< -o $@

$(mle_unit_test_all): $(mle_objects_no_main) $(mle_unit_test_objects) $(mle_unit_test_all).c $(mle_vendor_deps)
	$(CC) $(mle_cflags) -rdynamic $(mle_unit_test_all).c $(mle_objects_no_main) $(mle_unit_test_objects) $(mle_ldflags) $(mle_ldlibs) -ldl -o $@

$(mle_unit_tests): %: $(mle_unit_test_all)
	{ echo "#!/bin/sh"; echo "$(abspath $(mle_unit_test_all)) $(notdir $@)"; } >$@ && chmod +x $@

test: mle $(mle_unit_tests)
	./mle -v && export MLE=$$(pwd)/mle && $(MAKE) -C tests

sloc:
	find . -maxdepth 1 \
		'(' -name '*.c' -or -name '*.h' ')' \
		-not -name 'termbox2.h' \
		-exec cat {} ';' | wc -l

install: mle
	install -v -d $(DESTDIR)$(prefix)/bin
	install -v -m 755 mle $(DESTDIR)$(prefix)/bin/mle

uninstall:
	rm -v $(DESTDIR)$(prefix)/bin/mle

uscript:
	php uscript.inc.php >uscript.inc.c

clean_quick:
	rm -f mle $(mle_objects) $(mle_unit_test_objects) $(mle_unit_tests) $(mle_unit_test_all)

clean:
	rm -f mle $(mle_objects) $(mle_vendor_deps) $(mle_unit_test_objects) $(mle_unit_tests) $(mle_unit_test_all)
	$(MAKE) -C vendor clean

.NOTPARALLEL:

.PHONY: all test sloc install uscript clean
