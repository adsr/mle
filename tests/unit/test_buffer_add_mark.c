#include "test.h"

char *str = "hello\nworld";

void test(buffer_t *buf, mark_t *cur) {
    mark_t *mark;

    mark = buffer_add_mark(buf, NULL, 0);
    ASSERT("mark0bline", 1, mark->bline == buf->first_line ? 1 : 0);
    ASSERT("mark0col", 0, mark->col);

    mark = buffer_add_mark(buf, buf->first_line->next, 5);
    ASSERT("mark1bline", 1, mark->bline == buf->first_line->next ? 1 : 0);
    ASSERT("mark1col", 5, mark->col);
}

