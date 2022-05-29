#include "test.h"

char *str = "hello\nworld";

void test(buffer_t *buf, mark_t *cur) {
    char *data;
    bint_t data_len;

    bline_insert(buf->first_line, 5, "\n", 1, NULL);
    buffer_get(buf, &data, &data_len);
    ASSERT("nl", 0, strncmp(data, "hello\n\nworld", data_len));

    bline_insert(buf->first_line->next, 0, "my", 2, NULL);
    buffer_get(buf, &data, &data_len);
    ASSERT("my", 0, strncmp(data, "hello\nmy\nworld", data_len));

    bline_insert(buf->first_line->next, 0, "a", 1, NULL);
    buffer_get(buf, &data, &data_len);
    ASSERT("bol", 0, strncmp(data, "hello\namy\nworld", data_len));

    bline_insert(buf->first_line->next, 3, "'s", 2, NULL);
    buffer_get(buf, &data, &data_len);
    ASSERT("eol", 0, strncmp(data, "hello\namy's\nworld", data_len));

    bline_insert(buf->first_line->next, 6, " ", 1, NULL);
    buffer_get(buf, &data, &data_len);
    ASSERT("oob", 0, strncmp(data, "hello\namy's\n world", data_len));

    bline_insert(buf->last_line, buf->last_line->char_count, "\nno\xe2\x80\x8bspace", 11, NULL);
    // The following assertion relies on a UTF-8 locale. `nl_langinfo` may not
    // be available, so let's comment it out for now. (In practice, if locale
    // is, e.g., C, the zero-width space will show up as "?" with vwidth==1.)
    //
    // codeset = nl_langinfo(CODESET);
    // if (codeset && strcmp(codeset, "UTF-8") == 0) {
    //     ASSERT("0wv", 7, buf->last_line->char_vwidth);
    // }
    ASSERT("0wc", 8, buf->last_line->char_count);
    ASSERT("0wd", 10, buf->last_line->data_len);

    bline_insert(buf->last_line, buf->last_line->char_count, "\nnull\x00", 6, NULL);
    ASSERT("nulv", 5, buf->last_line->char_vwidth);
    ASSERT("nulc", 5, buf->last_line->char_count);
    ASSERT("nuld", 5, buf->last_line->data_len);
}
