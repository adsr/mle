SHELL=/bin/bash
DESTDIR?=/usr/bin/
mle_cflags:=$(CFLAGS) -D_GNU_SOURCE -Wall -Wno-missing-braces -g -I./mlbuf/ -I./termbox/src/
mle_ldlibs:=$(LDLIBS) -lm -lpcre
mle_objects:=$(patsubst %.c,%.o,$(wildcard *.c))
mle_static:=
termbox_cflags:=$(CFLAGS) -D_XOPEN_SOURCE -Wall -Wextra -std=gnu99 -O3 -Wno-unused-result
termbox_objects:=$(patsubst termbox/src/%.c,termbox/src/%.o,$(wildcard termbox/src/*.c))

all: mle

mle: $(mle_objects) ./mlbuf/libmlbuf.a ./termbox/src/libtermbox.a
	$(CC) $(mle_objects) $(mle_static) ./mlbuf/libmlbuf.a ./termbox/src/libtermbox.a $(mle_ldlibs) -o mle

mle_static: mle_static:=-static
mle_static: mle_ldlibs:=$(mle_ldlibs) -lpthread
mle_static: mle

$(mle_objects): %.o: %.c
	$(CC) -c $(mle_cflags) $< -o $@

./mlbuf/libmlbuf.a:
	$(MAKE) -C mlbuf

./termbox/src/libtermbox.a: $(termbox_objects)
	$(AR) rcs $@ $(termbox_objects)

$(termbox_objects): %.o: %.c
	$(CC) -c $(termbox_cflags) $< -o $@

test: mle test_mle
	$(MAKE) -C mlbuf test

test_mle: mle
	$(MAKE) -C tests && ./mle -v

sloc:
	find . -name '*.c' -or -name '*.h' | \
		grep -Pv '(termbox|test|ut)' | \
		xargs -rn1 cat | \
		wc -l

install: mle
	install -v -m 755 mle $(DESTDIR)

clean:
	rm -f *.o mle.bak.* gmon.out perf.data perf.data.old mle
	$(MAKE) -C mlbuf clean
	$(MAKE) -C tests clean
	rm -f termbox/src/*.o
	rm -f termbox/src/libtermbox.a

.PHONY: all mle_static test test_mle sloc install clean
