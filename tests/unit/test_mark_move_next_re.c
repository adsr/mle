#include "test.h"

MAIN("hi\nanna\nbanana",
    mark_move_beginning(cur);

    mark_move_next_re(cur, "an+a", strlen("an+a"));
    ASSERT("line1", buf->first_line->next, cur->bline);
    ASSERT("col1", 0, cur->col);

    mark_move_next_re(cur, "an+a", strlen("an+a"));
    ASSERT("line1again", buf->first_line->next, cur->bline);
    ASSERT("col1again", 0, cur->col);

    mark_move_next_re_nudge(cur, "an+a", strlen("an+a"));
    ASSERT("line2", buf->first_line->next->next, cur->bline);
    ASSERT("col2", 1, cur->col);

    mark_move_next_re_nudge(cur, "an+a", strlen("an+a"));
    ASSERT("line3", buf->first_line->next->next, cur->bline);
    ASSERT("col3", 3, cur->col);

    mark_move_beginning(cur);
    mark_move_next_re(cur, "hi", strlen("hi"));
    ASSERT("line4", buf->first_line, cur->bline);
    ASSERT("col4", 0, cur->col);
)
