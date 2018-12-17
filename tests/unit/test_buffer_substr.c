#include "test.h"

MAIN("hello\nworld",
    char *data;
    bint_t data_len;
    bint_t nchars;

    buffer_substr(buf, buf->first_line, 4, buf->first_line->next, 1, &data, &data_len, &nchars);
    ASSERT("datalen", 3, data_len);
    ASSERT("nchars", 3, nchars);
    ASSERT("substr", 0, strncmp("o\nw", data, data_len));
    free(data);
)
