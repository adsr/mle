#include "test.h"

char *str = "hello\nworld";

void test(buffer_t *buf, mark_t *cur) {
    bint_t offset;

    mark_move_end(cur);
    mark_get_offset(cur, &offset);
    ASSERT("offset", 11, offset);

    mark_move_beginning(cur);
    mark_get_offset(cur, &offset);
    ASSERT("offset", 0, offset);
}
