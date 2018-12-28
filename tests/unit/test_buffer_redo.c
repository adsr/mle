#include "test.h"

char *str = "hi";

void test(buffer_t *buf, mark_t *cur) {
    char *data;
    bint_t data_len;

    buffer_delete(buf, 0, 1);
    buffer_undo(buf);
    buffer_redo(buf);
    buffer_get(buf, &data, &data_len);
    ASSERT("del", 0, strncmp(data, "i", data_len));

    buffer_insert(buf, 0, "y", 1, NULL);
    buffer_undo(buf);
    buffer_redo(buf);
    buffer_get(buf, &data, &data_len);
    ASSERT("ins", 0, strncmp(data, "yi", data_len));
}
