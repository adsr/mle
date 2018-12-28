#include "test.h"

char *str = "hello\nworld";

void test(buffer_t *buf, mark_t *cur) {
    mark_move_beginning(cur);

    mark_move_col(cur, 5);
    ASSERT("col1", 5, cur->col);

    mark_move_col(cur, 6);
    ASSERT("oob", 5, cur->col);
}

