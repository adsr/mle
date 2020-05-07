#!/usr/bin/env bash

if [ -z "$MLE" ]; then
    echo "Expected MLE env var defined"
    exit 1
fi

$MLE -v

if [ "$(uname)" == "Darwin" ]; then
    tests=$(find $(pwd -P)/ -mindepth 2 -perm +111 -type f)
else
    tests=$(find $(pwd -P)/ -mindepth 2 -executable -type f)
fi

for t in $tests; do
    tshort=$(basename $t)
    tdir=$(dirname $t)
    tput bold 2>/dev/null; echo TEST $tshort; tput sgr0 2>/dev/null
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
