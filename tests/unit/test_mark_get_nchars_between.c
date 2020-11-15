#include "test.h"

char *str = "hello\nworld";

void test(buffer_t *buf, mark_t *cur) {
    mark_t *other;
    bint_t nchars;
    other = buffer_add_mark(buf, NULL, 0);
    mark_move_beginning(cur);
    mark_move_end(other);
    nchars = 0;
    mark_get_nchars_between(cur, other, &nchars);
    ASSERT("nchars", 11, nchars);
}
