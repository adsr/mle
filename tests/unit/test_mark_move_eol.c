#include "test.h"

char* str = "hello\nworld";

void test(buffer_t* buf, mark_t* cur) {
    mark_move_beginning(cur);
    mark_move_eol(cur);
    ASSERT("line", buf->first_line, cur->bline);
    ASSERT("col", 5, cur->col);
}
