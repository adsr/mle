DESTDIR?=/usr/local/bin/

mle_cflags:=$(CFLAGS) -D_GNU_SOURCE -Wall -Wextra -Wno-missing-braces -Wno-unused-parameter -Wno-unused-result -Wno-unused-function -g -O0 -I./mlbuf/ -I./termbox/src/ -I./uthash/src/ -I./lua
mle_ldlibs:=$(LDLIBS) -lm -lpcre
mle_ldflags:=$(LDFLAGS)
mle_objects:=$(patsubst %.c,%.o,$(wildcard *.c))
mle_static:=
termbox_cflags:=$(CFLAGS) -D_XOPEN_SOURCE -Wall -Wextra -Wno-unused-result -std=gnu99 -g -O0
termbox_objects:=$(patsubst termbox/src/%.c,termbox/src/%.o,$(wildcard termbox/src/*.c))
lua_objects:=$(patsubst lua/%.c,lua/%.o,$(wildcard lua/*.c))
lua_cflags:=$(CFLAGS) -DLUA_USE_POSIX -Wall -Wextra -std=gnu99 -g -O0

all: mle

mle: $(mle_objects) ./mlbuf/libmlbuf.a ./termbox/src/libtermbox.a ./lua/liblua.a
	$(CC) $(mle_cflags) $(mle_objects) $(mle_static) ./mlbuf/libmlbuf.a ./termbox/src/libtermbox.a ./lua/liblua.a $(mle_ldflags) $(mle_ldlibs) -o mle

mle_static: mle_static:=-static
mle_static: mle_ldlibs:=$(mle_ldlibs) -lpthread
mle_static: mle

$(mle_objects): %.o: %.c
	$(CC) -c $(mle_cflags) $< -o $@

./mlbuf/libmlbuf.a:
	$(MAKE) -C mlbuf

./termbox/src/libtermbox.a: $(termbox_objects)
	$(AR) rcs $@ $(termbox_objects)

./lua/liblua.a: $(lua_objects)
	$(AR) rcs $@ $(lua_objects)

$(termbox_objects): %.o: %.c
	$(CC) -c $(termbox_cflags) $< -o $@

$(lua_objects): %.o: %.c
	$(CC) -c $(lua_cflags) $< -o $@

test: mle test_mle
	$(MAKE) -C mlbuf test

test_mle: mle
	./mle -v && $(MAKE) -C tests

sloc:
	find . -name '*.c' -or -name '*.h' | \
		grep -Pv '(termbox|test|ut|lua)' | \
		xargs -rn1 cat | \
		wc -l

install: mle
	install -v -m 755 mle $(DESTDIR)

uscript:
	php uscript.inc.php >uscript.inc

clean:
	rm -f *.o mle.bak.* gmon.out perf.data perf.data.old mle
	$(MAKE) -C mlbuf clean
	rm -f lua/*.o lua/lua lua/liblua.a
	rm -f termbox/src/*.o termbox/src/libtermbox.a

.PHONY: all mle_static test test_mle sloc install uscript clean
