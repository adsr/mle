#include "test.h"

char* str = "hello\nworld";

void test(buffer_t* buf, mark_t* cur) {
    char *data;
    bint_t data_len;
    bint_t nchars;

    buffer_insert(buf, 0, "\xe4\xb8\x96\xe7\x95\x8c\n", 7, &nchars);
    buffer_get(buf, &data, &data_len);
    ASSERT("bol", 0, strncmp(data, "\xe4\xb8\x96\xe7\x95\x8c\nhello\nworld", data_len));
    ASSERT("bolnchars", 3, nchars);

    buffer_insert(buf, 3, "s", 1, &nchars);
    buffer_get(buf, &data, &data_len);
    ASSERT("3", 0, strncmp(data, "\xe4\xb8\x96\xe7\x95\x8c\nshello\nworld", data_len));
    ASSERT("3nchars", 1, nchars);

    buffer_insert(buf, 7, "\n\n", 2, &nchars);
    buffer_get(buf, &data, &data_len);
    ASSERT("nlnl", 0, strncmp(data, "\xe4\xb8\x96\xe7\x95\x8c\nshel\n\nlo\nworld", data_len));
    ASSERT("nlnlnchars", 2, nchars);

    buffer_insert(buf, 20, "!", 1, NULL);
    buffer_get(buf, &data, &data_len);
    ASSERT("oob", 0, strncmp(data, "\xe4\xb8\x96\xe7\x95\x8c\nshel\n\nlo\nworld!", data_len));
}

