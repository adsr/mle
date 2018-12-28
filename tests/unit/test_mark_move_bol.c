#include "test.h"

char *str = "hello\nworld";

void test(buffer_t *buf, mark_t *cur) {
    mark_move_end(cur);
    mark_move_bol(cur);
    ASSERT("line", buf->first_line->next, cur->bline);
    ASSERT("col", 0, cur->col);
}
