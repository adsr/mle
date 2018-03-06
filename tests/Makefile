test_scripts:=$(wildcard test_*.sh)
MLE?=../mle

all: run

run: $(test_scripts)
	total=0; \
	pass=0; \
	for t in $(test_scripts); do \
		tput bold; echo TEST $$t; tput sgr0; \
		MLE=$(MLE) ./$$t; ec=$$?; echo; \
		[ $$ec -eq 0 ] && pass=$$((pass+1)); \
		total=$$((total+1)); \
	done; \
	printf "Passed %d out of %d tests\n" $$pass $$total ; \
	[ $$pass -eq $$total ] || exit 1

.PHONY: all run
