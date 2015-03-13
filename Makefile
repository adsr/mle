TARGET=mle
CC=colorgcc

all: mle

mle: mlbuf.a
	$(CC) -Wall -pg -g -I./mlbuf/ -L./mlbuf/ *.c -o $@ ./mlbuf/libmlbuf.a -ltermbox -lpcre -lm

mlbuf.a:
	make -C mlbuf

clean:
	rm -f *.o
	rm -f gmon.out
	rm -f mle
	make -C mlbuf clean
