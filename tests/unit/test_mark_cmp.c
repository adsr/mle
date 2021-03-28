#include "test.h"

char *str = "hello\nworld";

void test(buffer_t *buf, mark_t *cur) {
    mark_t *other, *first, *second;
    int cmp;
    other = buffer_add_mark(buf, NULL, 0);

    mark_move_beginning(cur);
    mark_move_end(other);
    cmp = mark_cmp(cur, other, &first, &second);
    ASSERT("a_cmp", -1, cmp);
    ASSERT("a_first", cur, first);
    ASSERT("a_second", other, second);

    mark_move_end(cur);
    mark_move_beginning(other);
    cmp = mark_cmp(cur, other, &first, &second);
    ASSERT("b_cmp", 1, cmp);
    ASSERT("b_first", other, first);
    ASSERT("b_second", cur, second);

    mark_move_beginning(cur);
    mark_move_beginning(other);
    cmp = mark_cmp(cur, other, &first, &second);
    ASSERT("c_cmp", 0, cmp);
}
