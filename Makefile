prefix?=/usr/local

mle_cflags:=-std=c99 -Wall -Wextra -pedantic -Wno-pointer-arith -Wno-unused-result -Wno-unused-parameter -g -O3 -D_GNU_SOURCE -I.  $(CFLAGS)
mle_ldflags:=$(LDFLAGS)
mle_libs:=-lpcre -ltermbox -llua5.3
mle_ldlibs:=-lm $(LDLIBS)
mle_objects:=$(patsubst %.c,%.o,$(wildcard *.c))
mle_objects_no_main:=$(filter-out main.o,$(mle_objects))
mle_func_tests:=$(wildcard tests/func/test_*.sh))
mle_unit_tests:=$(patsubst %.c,%,$(wildcard tests/unit/*.c))
mle_link_default:=-Wl,-Bdynamic
mle_vendor_deps:=

ifdef mle_static
  mle_static_var:=-static
  mle_link_default:=-static
endif

ifdef mle_vendor
  mle_ldlibs:=-Wl,-Bstatic $(mle_libs) $(mle_link_default) $(mle_ldlibs)
  mle_ldflags:=-Lvendor/pcre/.libs -Lvendor/termbox/src -Lvendor/lua $(mle_ldflags)
  mle_cflags:=-Ivendor/pcre -Ivendor/termbox/src -Ivendor -Ivendor/uthash/src $(mle_cflags)
  mle_vendor_deps:=./vendor/pcre/.libs/libpcre.a ./vendor/termbox/src/libtermbox.a ./vendor/lua/liblua5.3.a
else
  mle_ldlibs:=$(mle_libs) $(mle_ldlibs)
endif

all: mle

mle: $(mle_objects) $(mle_vendor_deps)
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
	install -D -v -m 755 mle $(DESTDIR)$(prefix)/bin/mle

uscript:
	php uscript.inc.php >uscript.inc

clean:
	rm -f mle $(mle_objects) $(mle_vendor_deps) $(mle_unit_tests)
	$(MAKE) -C vendor clean

.PHONY: all test sloc install uscript clean
