#include "test.h"

char *str = "hello\nworld";

void test(buffer_t *buf, mark_t *cur) {
    mark_t *other;
    other = buffer_add_mark(buf, NULL, 0);
    mark_move_beginning(cur);
    mark_move_end(other);
    mark_delete_between_mark(cur, other);
    ASSERT("count", 0, buf->byte_count);
}
