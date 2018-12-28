#include "test.h"

char *str = "hello\nworld";

void test(buffer_t *buf, mark_t *cur) {
    mark_t *other;
    other = buffer_add_mark(buf, NULL, 0);

    mark_move_beginning(cur);
    mark_move_end(other);
    ASSERT("gt", 1, mark_is_gt(other, cur));
    ASSERT("ngt1", 0, mark_is_gt(cur, other));

    mark_move_beginning(other);
    ASSERT("ngt2", 0, mark_is_gt(other, cur));
    ASSERT("ngt3", 0, mark_is_gt(cur, other));
}
