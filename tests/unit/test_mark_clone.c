#include "test.h"

char* str = "hello\nworld";

void test(buffer_t* buf, mark_t* cur) {
    mark_t* other;
    mark_clone(cur, &other);
    ASSERT("neq", 1, other != cur ? 1 : 0);
    ASSERT("next", 1, other == cur->next ? 1 : 0);
    ASSERT("line", cur->bline, other->bline);
    ASSERT("col", cur->col, other->col);
}
