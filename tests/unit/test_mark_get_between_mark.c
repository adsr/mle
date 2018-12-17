#include "test.h"

MAIN("hello\nworld",
    char* str;
    bint_t str_len;
    mark_t* other;
    other = buffer_add_mark(buf, NULL, 0);
    mark_move_beginning(cur);
    mark_move_end(other);
    mark_get_between_mark(cur, other, &str, &str_len);
    ASSERT("len", 11, str_len);
    ASSERT("eq", 0, strncmp(str, "hello\nworld", str_len));
)
