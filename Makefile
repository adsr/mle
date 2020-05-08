prefix?=/usr/local

mle_cflags:=-std=c99 -Wall -Wextra -pedantic -Wno-pointer-arith -Wno-unused-result -Wno-unused-parameter -g -O3 -D_GNU_SOURCE -I.  $(CFLAGS)
mle_ldflags:=$(LDFLAGS)
mle_dynamic_libs:=-lpcre -ltermbox -llua5.3
mle_static_libs:=vendor/pcre/.libs/libpcre.a vendor/termbox/src/libtermbox.a vendor/lua/liblua5.3.a
mle_ldlibs:=-lm $(LDLIBS)
mle_objects:=$(patsubst %.c,%.o,$(wildcard *.c))
mle_objects_no_main:=$(filter-out main.o,$(mle_objects))
mle_func_tests:=$(wildcard tests/func/test_*.sh))
mle_unit_tests:=$(patsubst %.c,%,$(wildcard tests/unit/*.c))
mle_vendor_deps:=
mle_static_var:=

ifdef mle_static
  mle_static_var:=-static
endif

ifdef mle_vendor
  mle_ldlibs:=$(mle_static_libs) $(mle_ldlibs)
  mle_cflags:=-Ivendor/pcre -Ivendor/termbox/src -Ivendor -Ivendor/uthash/src $(mle_cflags)
  mle_vendor_deps:=$(mle_static_libs)
else
  mle_ldlibs:=$(mle_dynamic_libs) $(mle_ldlibs)
endif

all: mle

mle: $(mle_vendor_deps) $(mle_objects)
	$(CC) $(mle_static_var) $(mle_cflags) $(mle_objects) $(mle_ldflags) $(mle_ldlibs) -o mle

$(mle_objects): %.o: %.c
	$(CC) -c $(mle_cflags) $< -o $@

$(mle_vendor_deps):
	$(MAKE) -C vendor

$(mle_unit_tests): %: %.c
	$(CC) $(mle_cflags) $(mle_objects_no_main) $(mle_ldflags) $(mle_ldlibs) $< -o $@

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

clean:
	rm -f mle $(mle_objects) $(mle_vendor_deps) $(mle_unit_tests)
	$(MAKE) -C vendor clean

.PHONY: all test sloc install uscript clean
