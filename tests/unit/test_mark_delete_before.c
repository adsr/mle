#include "test.h"

MAIN("hello\nworld",
    char* data;
    bint_t data_len;
    mark_move_end(cur);
    mark_delete_before(cur, 1);
    buffer_get(buf, &data, &data_len);
    ASSERT("delb", 0, strncmp("hello\nworl", data, data_len));
)
