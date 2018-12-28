#include "test.h"

char* str = "Ervin won gold at age 35!";

void test(buffer_t* buf, mark_t* cur) {
    int ovector[6] = {0};
    int rc = 0;
    bline_t* bline;
    bint_t col;
    bint_t nchars;

    mark_move_beginning(cur);

    ASSERT("set", MLBUF_OK, mark_set_pcre_capture(&rc, ovector, 6));

    mark_find_next_re(cur, "age ([0-9]+)", strlen("age ([0-9]+)"), &bline, &col, &nchars);
    ASSERT("bline", cur->bline, bline);
    ASSERT("col", 18, col);
    ASSERT("nchars", 6, nchars);
    ASSERT("rc", 2, rc);
    ASSERT("vec1a", 18, ovector[0]);
    ASSERT("vec1b", 24, ovector[1]);
    ASSERT("vec2a", 22, ovector[2]);
    ASSERT("vec2b", 24, ovector[3]);
}
