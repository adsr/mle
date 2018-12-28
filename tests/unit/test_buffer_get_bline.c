#include "test.h"

char* str = "0\n1\n2\n3\n4\n";

void test(buffer_t* buf, mark_t* cur) {
    bint_t i;
    bline_t* line;
    bline_t* line2;
    for (line = buf->first_line, i = 0; line; line = line->next, i += 1) {
        buffer_get_bline(buf, i, &line2);
        ASSERT("line", line, line2);
    }
}
