#include "test.h"

MAIN("hello\nworld",
    char *data;
    bint_t data_len;

    bline_insert(buf->first_line, 5, "\n", 1, NULL);
    buffer_get(buf, &data, &data_len);
    ASSERT("nl", 0, strncmp(data, "hello\n\nworld", data_len));

    bline_insert(buf->first_line->next, 0, "my", 2, NULL);
    buffer_get(buf, &data, &data_len);
    ASSERT("my", 0, strncmp(data, "hello\nmy\nworld", data_len));

    bline_insert(buf->first_line->next, 0, "a", 1, NULL);
    buffer_get(buf, &data, &data_len);
    ASSERT("bol", 0, strncmp(data, "hello\namy\nworld", data_len));

    bline_insert(buf->first_line->next, 3, "'s", 2, NULL);
    buffer_get(buf, &data, &data_len);
    ASSERT("eol", 0, strncmp(data, "hello\namy's\nworld", data_len));

    bline_insert(buf->first_line->next, 6, " ", 1, NULL);
    buffer_get(buf, &data, &data_len);
    ASSERT("oob", 0, strncmp(data, "hello\namy's\n world", data_len));
)
