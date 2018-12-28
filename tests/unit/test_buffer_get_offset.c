#include "test.h"

char *str = "hello\nworld";

void test(buffer_t *buf, mark_t *cur) {
    bint_t offset;

    buffer_get_offset(buf, buf->first_line, 0, &offset);
    ASSERT("f0", 0, offset);

    buffer_get_offset(buf, buf->first_line, 1, &offset);
    ASSERT("f1", 1, offset);

    buffer_get_offset(buf, buf->first_line, 5, &offset);
    ASSERT("f5", 5, offset);

    buffer_get_offset(buf, buf->first_line->next, 0, &offset);
    ASSERT("n0", 6, offset);

    buffer_get_offset(buf, buf->first_line->next, 6, &offset);
    ASSERT("noob", 11, offset);
}
