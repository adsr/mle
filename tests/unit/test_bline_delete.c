#include "test.h"

char* str = "hello\nworld";

void test(buffer_t* buf, mark_t* cur) {
    char *data;
    bint_t data_len;

    bline_delete(buf->first_line, 0, 1);
    buffer_get(buf, &data, &data_len);
    ASSERT("bol", 0, strncmp(data, "ello\nworld", data_len));

    bline_delete(buf->first_line, 0, 0);
    buffer_get(buf, &data, &data_len);
    ASSERT("noop", 0, strncmp(data, "ello\nworld", data_len));

    bline_delete(buf->first_line->next, 4, 1);
    buffer_get(buf, &data, &data_len);
    ASSERT("eol", 0, strncmp(data, "ello\nworl", data_len));

    bline_delete(buf->first_line, 1, 2);
    buffer_get(buf, &data, &data_len);
    ASSERT("mid", 0, strncmp(data, "eo\nworl", data_len));

    bline_delete(buf->first_line, 4, 1);
    buffer_get(buf, &data, &data_len);
    ASSERT("oob", 0, strncmp(data, "eo\nwrl", data_len));

    bline_delete(buf->first_line, 2, 3);
    buffer_get(buf, &data, &data_len);
    ASSERT("eatnl", 0, strncmp(data, "eol", data_len));
}
