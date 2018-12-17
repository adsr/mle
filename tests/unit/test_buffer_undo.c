#include "test.h"

MAIN("hi",
    char* data;
    bint_t data_len;

    buffer_delete(buf, 0, 1);
    buffer_undo(buf);
    buffer_get(buf, &data, &data_len);
    ASSERT("del", 0, strncmp(data, "hi", data_len));

    buffer_insert(buf, 0, "c", 1, NULL);
    buffer_undo(buf);
    buffer_get(buf, &data, &data_len);
    ASSERT("ins", 0, strncmp(data, "hi", data_len));
)
