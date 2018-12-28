#include "test.h"

char* str = "hello\nworld";

void test(buffer_t* buf, mark_t* cur) {
    mark_move_beginning(cur);

    mark_move_by(cur, 1);
    ASSERT("col1", 1, cur->col);

    mark_move_by(cur, 4);
    ASSERT("col2", 5, cur->col);

    mark_move_by(cur, 1);
    ASSERT("col3", 0, cur->col);
    ASSERT("line3", buf->first_line->next, cur->bline);

    mark_move_by(cur, -1);
    ASSERT("col4", 5, cur->col);
    ASSERT("line4", buf->first_line, cur->bline);
}
