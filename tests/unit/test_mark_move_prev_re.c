#include "test.h"

MAIN("hi\nanna\nbanana",
    mark_move_end(cur);

    mark_move_prev_re(cur, "an+a", strlen("an+a"));
    ASSERT("line3", buf->first_line->next->next, cur->bline);
    ASSERT("col3", 3, cur->col);

    mark_move_prev_re(cur, "an+a", strlen("an+a"));
    ASSERT("line2", buf->first_line->next->next, cur->bline);
    ASSERT("col2", 1, cur->col);

    mark_move_prev_re(cur, "an+a", strlen("an+a"));
    ASSERT("line1", buf->first_line->next, cur->bline);
    ASSERT("col1", 0, cur->col);

    mark_move_end(cur);
    mark_move_prev_re(cur, "^", strlen("^"));
    ASSERT("cflex_line", buf->first_line->next->next, cur->bline);
    ASSERT("cflex_col", 0, cur->col);
)
