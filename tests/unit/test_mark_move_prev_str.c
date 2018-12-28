#include "test.h"

char *str = "hi\nana\nbanana";

void test(buffer_t *buf, mark_t *cur) {
    mark_move_end(cur);

    mark_move_prev_str(cur, "ana", strlen("ana"));
    ASSERT("line3", buf->first_line->next->next, cur->bline);
    ASSERT("col3", 3, cur->col);

    mark_move_prev_str(cur, "ana", strlen("ana"));
    ASSERT("line2", buf->first_line->next->next, cur->bline);
    ASSERT("col2", 1, cur->col);

    mark_move_prev_str(cur, "ana", strlen("ana"));
    ASSERT("line1", buf->first_line->next, cur->bline);
    ASSERT("col1", 0, cur->col);
}
