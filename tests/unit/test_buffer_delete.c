#include "test.h"

MAIN("hello\nworld",
    char *data;
    bint_t data_len;

    buffer_delete(buf, 0, 1);
    buffer_get(buf, &data, &data_len);
    ASSERT("bol", 0, strncmp(data, "ello\nworld", data_len));

    buffer_delete(buf, 4, 1);
    buffer_get(buf, &data, &data_len);
    ASSERT("nl", 0, strncmp(data, "elloworld", data_len));

    buffer_delete(buf, 7, 3);
    buffer_get(buf, &data, &data_len);
    ASSERT("oob", 0, strncmp(data, "ellowor", data_len));

    buffer_delete(buf, 0, 7);
    buffer_get(buf, &data, &data_len);
    ASSERT("all", 0, strncmp(data, "", data_len));
)

