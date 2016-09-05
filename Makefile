SHELL=/bin/bash
DESTDIR?=/usr/bin/
mle_cflags:=$(CFLAGS) -D_GNU_SOURCE -Wall -Wno-missing-braces -g -I./mlbuf/ -I./termbox/src/
mle_ldlibs:=$(LDLIBS) -lm -lpcre
mle_objects:=$(patsubst %.c,%.o,$(wildcard *.c))
mle_static:=

all: mle

mle: $(mle_objects) ./mlbuf/libmlbuf.a ./termbox/build/src/libtermbox.a
	$(CC) $(mle_objects) $(mle_static) ./mlbuf/libmlbuf.a ./termbox/build/src/libtermbox.a $(mle_ldlibs) -o mle

mle_static: mle_static:=-static
mle_static: mle_ldlibs:=$(mle_ldlibs) -lpthread
mle_static: mle

$(mle_objects): %.o: %.c
	$(CC) -c $(mle_cflags) $< -o $@

./mlbuf/libmlbuf.a:
	$(MAKE) -C mlbuf

./termbox/build/src/libtermbox.a:
	pushd termbox && ./waf configure &>/dev/null && ./waf &>/dev/null && popd

test: mle
	$(MAKE) -C mlbuf test
	./mle -v

install: mle
	install -v -m 755 mle $(DESTDIR)

clean:
	rm -f *.o mle.bak.* gmon.out perf.data perf.data.old mle
	$(MAKE) -C mlbuf clean
	pushd termbox && ./waf clean && popd

.PHONY: all mle_static test install clean
