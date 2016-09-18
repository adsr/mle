#!/bin/bash

mle='../mle'
[ -n "$mle_stdin" ]  || mle_stdin=""
[ -n "$mle_expect" ] || mle_expect=""
[ -n "$mle_args" ]   || mle_args="-N"

if [ -n "$mle_lel" ]; then
    mle_cmd='cmd_lel'
    mle_param=$mle_lel
fi

if [ -n "$mle_cmd" ]; then
    mle_kbind="$mle_cmd,t,$mle_param"
fi

echo -n "$mle_stdin" >test.data
mle_stdout=$($mle -N -Ktest,0,0 -k"$mle_kbind" -ntest -M't t' -pt <test.data)

if [ "$mle_stdout" == "$mle_expect" ]; then
    echo "  $(tput setaf 2)OK$(tput sgr0)  $mle_tname"
else
    echo "  $mle -N -Ktest,0,0 -k$(printf "%q" "$mle_kbind") -ntest -M't t' -pt <test.data"
    echo "  $(tput setaf 1)ERR$(tput sgr0) $mle_tname"
    echo "    expected: $(echo $mle_expect | tr -d '\n')"
    echo "    observed: $(echo $mle_stdout | tr -d '\n')"
    exit 1
fi

mle_tname=''
