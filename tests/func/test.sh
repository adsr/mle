#!/usr/bin/env bash
if [ -n "$skip" ]; then
    echo -e "  \x1b[33mSKIP\x1b[0m $skip"
    unset skip
else
    actual=$(
        $MLE \
        -N \
        -H1 \
        -Qd \
        -K test_kmap,,1 \
        -k cmd_quit_without_saving,f12, \
        -n test_kmap \
        -M "test_macro $macro f12" \
        -p test_macro \
        "${extra_opts[@]}" \
        2>&1 >/dev/null
    );
    exit_code=$?

    if [ "$exit_code" -ne 0 ]; then
        echo -e "  \x1b[31mERR \x1b[0m nonzero_exit_code=$exit_code\n\n$actual"
        exit 1
    fi

    for assert_name in "${!expected[@]}"; do
        expected_re="${expected[$assert_name]}"
        if grep -Eq "$expected_re" <<<"$actual"; then
            echo -e "  \x1b[32mOK  \x1b[0m $assert_name"
        else
            echo -e "  \x1b[31mERR \x1b[0m $assert_name expected=$expected_re\n\n$actual"
            exit 1
        fi
    done

    for assert_name in "${!not_expected[@]}"; do
        not_expected_re="${not_expected[$assert_name]}"
        if ! grep -Eq "$not_expected_re" <<<"$actual"; then
            echo -e "  \x1b[32mOK  \x1b[0m $assert_name"
        else
            echo -e "  \x1b[31mERR \x1b[0m $assert_name not_expected=$not_expected_re\n\n$actual"
            exit 1
        fi
    done

    unset expected
    unset not_expected
fi
