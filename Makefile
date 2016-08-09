SHELL=/bin/bash
CC=$(shell if which colorgcc >/dev/null 2>&1; then echo colorgcc; else echo gcc; fi)
BINDIR=/usr/bin

all: mle

mle: *.c *.h ./mlbuf/libmlbuf.a ./termbox/build/src/libtermbox.a
	$(CC) -D_GNU_SOURCE -Wall -Wno-missing-braces -g -I./mlbuf/ -I./termbox/src/ *.c -o $@ -static ./mlbuf/libmlbuf.a ./termbox/build/src/libtermbox.a -lm -lpcre -lpthread

./mlbuf/libmlbuf.a:
	make -C mlbuf

./termbox/build/src/libtermbox.a:
	pushd termbox; ./waf configure && ./waf; popd

test: mle
	make -C mlbuf test
	./mle -v

install: mle
	cp -vf mle $(BINDIR)/mle && chown -v 755 $(BINDIR)/mle

clean:
	rm -f *.o
	rm -f mle.bak.*
	rm -f gmon.out perf.data perf.data.old
	rm -f mle
	make -C mlbuf clean
	pushd termbox; ./waf clean; popd

.PHONY: all mle test install clean
