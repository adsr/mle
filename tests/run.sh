#!/bin/bash

if [ -z "$MLE" ]; then
    echo "Expected MLE env var defined"
    exit 1
fi

$MLE -v

for t in $(find . -mindepth 2 -executable -type f); do
    tshort=$(basename $t)
    tfull=$(readlink -f $t)
    tdir=$(dirname $tfull)
    tput bold; echo TEST $tshort; tput sgr0
    pushd $tdir &>/dev/null
    ./$tshort
    ec=$?
    popd &>/dev/null
    echo
    [ $ec -eq 0 ] && pass=$((pass+1))
    total=$((total+1))
done
printf "Passed %d out of %d tests\n" $pass $total
[ $pass -eq $total ] || exit 1
