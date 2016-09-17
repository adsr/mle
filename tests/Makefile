SHELL=/bin/bash
test_shs:=$(wildcard test_*.sh)

all: run

run: $(test_shs) ../mle
	total=0; \
	pass=0; \
	for t in $(test_shs); do \
		tput bold; echo TEST $$t; tput sgr0; \
		./$$t; ec=$$?; echo; \
		[ $$ec -eq 0 ] && pass=$$((pass+1)); \
		total=$$((total+1)); \
	done; \
	printf "Passed %d out of %d tests\n" $$pass $$total ; \
	[ $$pass -eq $$total ] || exit 1

../mle:
	$(MAKE) -C ..

clean:
	rm -f test.data

.PHONY: all run clean
