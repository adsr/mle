#include "test.h"

MAIN("hello\nworld",
    mark_move_offset(cur, 11);
    ASSERT("col1", 5, cur->col);
    ASSERT("line1", buf->first_line->next, cur->bline);

    mark_move_offset(cur, 6);
    ASSERT("col2", 0, cur->col);
    ASSERT("line2", buf->first_line->next, cur->bline);

    mark_move_offset(cur, 12);
    ASSERT("oobcol", 5, cur->col);
    ASSERT("oobline", buf->first_line->next, cur->bline);
)

