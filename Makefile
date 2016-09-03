export SHELL=/bin/bash
export CC=$(shell if which colorgcc >/dev/null 2>&1; then echo colorgcc; else echo gcc; fi)
export BINDIR=/usr/bin

all: mle

# TODO clean this crap up

mle: *.c *.h ./mlbuf/libmlbuf.a ./termbox/build/src/libtermbox.a
	php uscript_code_gen.php >uscript_code_gen.inc
	$(CC) -D_GNU_SOURCE -Wall -Wno-missing-braces -g -I./mlbuf/ -I./termbox/src/ -I./jsmn/ *.c jsmn/*.c -o mle ./mlbuf/libmlbuf.a ./termbox/build/src/libtermbox.a -lm -lpcre

mle_static: *.c *.h ./mlbuf/libmlbuf.a ./termbox/build/src/libtermbox.a
	php uscript_code_gen.php >uscript_code_gen.inc
	$(CC) -D_GNU_SOURCE -Wall -Wno-missing-braces -g -I./mlbuf/ -I./termbox/src/ -I./jsmn/ *.c jsmn/*.c -o mle -static ./mlbuf/libmlbuf.a ./termbox/build/src/libtermbox.a -lm -lpcre -lpthread

./mlbuf/libmlbuf.a:
	$(MAKE) -C mlbuf

./termbox/build/src/libtermbox.a:
	pushd termbox && ./waf configure && ./waf && popd

test: mle
	$(MAKE) -C mlbuf test
	./mle -v

install: mle
	install -v -m 755 mle $(BINDIR)

clean:
	rm -f *.o
	rm -f mle.bak.*
	rm -f gmon.out perf.data perf.data.old
	rm -f mle
	$(MAKE) -C mlbuf clean
	pushd termbox && ./waf clean && popd

.PHONY: all test mle_static install clean
