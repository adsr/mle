#include "test.h"

MAIN("hello\nworld",
    bint_t offset;

    mark_move_end(cur);
    mark_get_offset(cur, &offset);
    ASSERT("offset", 11, offset);

    mark_move_beginning(cur);
    mark_get_offset(cur, &offset);
    ASSERT("offset", 0, offset);
)
