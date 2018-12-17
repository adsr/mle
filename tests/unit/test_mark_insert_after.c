#include "test.h"

MAIN("hello\nworld",
    char* data;
    bint_t data_len;
    mark_move_beginning(cur);
    mark_insert_after(cur, "s", 1);
    buffer_get(buf, &data, &data_len);
    ASSERT("insa", 0, strncmp("shello\nworld", data, data_len));
    ASSERT("col", 0, cur->col);
)
