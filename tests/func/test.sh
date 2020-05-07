#!/usr/bin/env bash

actual=$(
    $MLE \
    -N \
    -H1 \
    -Qd \
    -K test_kmap,,1 \
    -k cmd_quit_without_saving,F12, \
    -n test_kmap \
    -M "test_macro $macro F12" \
    -p test_macro \
    $extra_opts \
    2>&1 >/dev/null
);

if [ "$(uname)" == "Darwin" ]; then
    grep_args="-Eq"
else
    grep_args="-Pq"
fi

for testname in "${!expected[@]}"; do
    expected_re="${expected[$testname]}"
    if grep $grep_args "$expected_re" <<<"$actual"; then
        echo -e "  \x1b[32mOK \x1b[0m $testname"
    else
        echo -e "  \x1b[31mERR\x1b[0m $testname expected=$expected_re\n\n$actual"
        exit 1
    fi
done

unset expected
