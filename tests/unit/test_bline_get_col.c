#include "test.h"

MAIN("h\xc3\x85llo \xe4\xb8\x96\xe7\x95\x8c", // "hallo ww"
//    01   2   34567   8   9   10  11  12
//    01       23456           7
    bint_t col;
    MLBUF_BLINE_ENSURE_CHARS(buf->first_line);
    ASSERT("len", 8, buf->first_line->char_count);

    bline_get_col(buf->first_line, 0, &col);
    ASSERT("0", 0, col);

    bline_get_col(buf->first_line, 1, &col);
    ASSERT("1", 1, col);

    bline_get_col(buf->first_line, 2, &col);
    ASSERT("2", 1, col);

    bline_get_col(buf->first_line, 3, &col);
    ASSERT("3", 2, col);

    bline_get_col(buf->first_line, 7, &col);
    ASSERT("7", 6, col);

    bline_get_col(buf->first_line, 9, &col);
    ASSERT("9", 6, col);

    bline_get_col(buf->first_line, 10, &col);
    ASSERT("10", 7, col);

    bline_get_col(buf->first_line, 11, &col);
    ASSERT("11", 7, col);

    bline_get_col(buf->first_line, 13, &col);
    ASSERT("13", 8, col);
)

