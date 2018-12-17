#include "test.h"

MAIN("hello\nworld",
    mark_move_beginning(cur);
    mark_move_eol(cur);
    ASSERT("line", buf->first_line, cur->bline);
    ASSERT("col", 5, cur->col);
)
