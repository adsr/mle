#include "test.h"

MAIN("hello\nworld",
    mark_move_beginning(cur);

    mark_move_to(cur, 0, 0);
    ASSERT("lin1", buf->first_line, cur->bline);
    ASSERT("col1", 0, cur->col);

    mark_move_to(cur, 0, 5);
    ASSERT("lin2", buf->first_line, cur->bline);
    ASSERT("col2", 5, cur->col);

    mark_move_to(cur, 1, 0);
    ASSERT("lin3", buf->first_line->next, cur->bline);
    ASSERT("col3", 0, cur->col);

    mark_move_to(cur, 1, 3);
    ASSERT("lin4", buf->first_line->next, cur->bline);
    ASSERT("col4", 3, cur->col);

    mark_move_to(cur, 1, 6);
    ASSERT("oob1lin", buf->first_line->next, cur->bline);
    ASSERT("oob1col", 5, cur->col);

    mark_move_to(cur, 2, 0);
    ASSERT("oob2lin", buf->first_line->next, cur->bline);
    ASSERT("oob2col", 0, cur->col);

    mark_move_to(cur, 2, 1);
    ASSERT("oob3lin", buf->first_line->next, cur->bline);
    ASSERT("oob3col", 1, cur->col);
)
