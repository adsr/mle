TARGET=mle
CC=colorgcc

all: mle

mle: mlbuf.a
	$(CC) -g -Wall -I./mlbuf/ -L./mlbuf/ *.c -o $@ -lmlbuf -ltermbox

mlbuf.a:
	make -C mlbuf

clean:
	rm -f *.o
	rm -f mle
	make -C mlbuf clean
