#include "test.h"

MAIN("hello\nworld",
    mark_move_beginning(cur);

    mark_move_col(cur, 5);
    ASSERT("col1", 5, cur->col);

    mark_move_col(cur, 6);
    ASSERT("oob", 5, cur->col);
)

