#include "test.h"

#define comma ,

char* str = "he\tllo\t\t";

void test(buffer_t* buf, mark_t* cur) {
    bint_t i;
    bint_t char_vcols_4[8] = {0 comma  1 comma  2 comma  4 comma  5 comma  6 comma  7 comma  8};
    bint_t char_vcols_2a[8] = {0 comma  1 comma  2 comma  4 comma  5 comma  6 comma  7 comma  8};
    bint_t char_vcols_2b[9] = {0 comma  1 comma  2 comma  4 comma  5 comma  6 comma  7 comma  8 comma  10};

    buffer_set_tab_width(buf, 4);
    // [he  llo     ] // char_vcol
    // [  t    tt   ] // tabs
    ASSERT("count4", 8, buf->first_line->char_count);
    ASSERT("width4", 12, buf->first_line->char_vwidth);
    for (i = 0; i < buf->first_line->char_count; i++) {
        ASSERT("vcol4", char_vcols_4[i], buf->first_line->chars[i].vcol);
    }

    buffer_set_tab_width(buf, 2);
    // [he  llo   ] // char_vcol
    // [  t    tt ] // tabs
    ASSERT("count2a", 8, buf->first_line->char_count);
    ASSERT("width2a", 10, buf->first_line->char_vwidth);
    for (i = 0; i < buf->first_line->char_count; i++) {
        ASSERT("vcol2a", char_vcols_2a[i], buf->first_line->chars[i].vcol);
    }

    bline_insert(buf->first_line, 4, "\t", 1, NULL);
    // [he  l lo    ] // char_vcol
    // [  t  t  t t ] // tabs
    ASSERT("count2b", 9, buf->first_line->char_count);
    ASSERT("width2b", 12, buf->first_line->char_vwidth);
    for (i = 0; i < buf->first_line->char_count; i++) {
        ASSERT("vcol2b", char_vcols_2b[i], buf->first_line->chars[i].vcol);
    }
}
