#include "test.h"

char *str = "[ bracket [ test ] ] xyz";

void test(buffer_t *buf, mark_t *cur) {
    mark_move_beginning(cur);
    mark_move_bracket_pair(cur, 1024);
    ASSERT("col1", 19, cur->col);
    mark_move_by(cur, -2);
    mark_move_bracket_pair(cur, 1024);
    ASSERT("col2", 10, cur->col);
}
