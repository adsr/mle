#include "test.h"

MAIN("hi\nworld",
    mark_move_end(cur);

    mark_move_vert(cur, 1);
    ASSERT("oob", buf->first_line->next, cur->bline);

    mark_move_vert(cur, 0);
    ASSERT("noop", buf->first_line->next, cur->bline);

    mark_move_vert(cur, -1);
    ASSERT("noop", buf->first_line, cur->bline);
    ASSERT("col", 2, cur->col);
    ASSERT("tcol", 5, cur->target_col);
)
