#include "test.h"

char* str = "hello\nworld";

void test(buffer_t* buf, mark_t* cur) {
    mark_t* other;
    other = buffer_add_mark(buf, NULL, 0);
    mark_move_beginning(cur);
    mark_move_end(other);
    mark_swap_with_mark(cur, other);
    ASSERT("end", buf->first_line->next, cur->bline);
    ASSERT("endcol", 5, cur->col);
    ASSERT("beg", buf->first_line, other->bline);
    ASSERT("begcol", 0, other->col);
}
