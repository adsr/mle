#include "test.h"

char *str = "  hello {\n    world\n  }";

void test(buffer_t *buf, mark_t *cur) {
    mark_t *other;
    other = buffer_add_mark(buf, NULL, 0);
    mark_move_next_str(cur, "{", 1);
    mark_move_by(cur, 1);
    mark_move_next_str(other, "}", 1);
    mark_delete_between(cur, other);
    ASSERT("lnct", 1, buf->line_count);
    ASSERT("data", 0, strncmp("  hello {}", buf->first_line->data, buf->first_line->data_len));
    ASSERT("clin", 0, cur->bline->line_index);
    ASSERT("ccol", 9, cur->col);
    ASSERT("olin", 0, cur->bline->line_index);
    ASSERT("ocol", 9, cur->col);
    mark_destroy(other);
}
