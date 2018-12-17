#include "test.h"

MAIN("hello\nworld",
    char* data;
    bint_t data_len;

    buffer_get(buf, &data, &data_len);
    ASSERT("len", 11, data_len);
    ASSERT("get", 0, strncmp(data, "hello\nworld", data_len));
)
