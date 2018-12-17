#include "test.h"

MAIN("hello\nworld",
    mark_move_end(cur);
    mark_move_bol(cur);
    ASSERT("line", buf->first_line->next, cur->bline);
    ASSERT("col", 0, cur->col);
)
