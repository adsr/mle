#include "test.h"

char *str = "hi";

void test(buffer_t *buf, mark_t *cur) {
    char *data;
    bint_t data_len;
    int action_group;

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

    action_group = 0;
    buffer_set_action_group_ptr(buf, &action_group);

    buffer_insert(buf, 2, "e", 1, NULL);
    buffer_insert(buf, 3, "ld", 2, NULL);
    action_group += 1;
    buffer_undo_action_group(buf);
    buffer_redo_action_group(buf);
    buffer_get(buf, &data, &data_len);
    ASSERT("ins_group", 0, strncmp(data, "yield", data_len));
}
