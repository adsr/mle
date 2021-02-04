#include "test.h"

char *str = "hi";

void test(buffer_t *buf, mark_t *cur) {
    char *data;
    bint_t data_len;
    int action_group;

    buffer_delete(buf, 0, 1);
    buffer_undo(buf);
    buffer_get(buf, &data, &data_len);
    ASSERT("del", 0, strncmp(data, "hi", data_len));

    buffer_insert(buf, 0, "c", 1, NULL);
    buffer_undo(buf);
    buffer_get(buf, &data, &data_len);
    ASSERT("ins", 0, strncmp(data, "hi", data_len));

    action_group = 0;
    buffer_set_action_group_ptr(buf, &action_group);

    buffer_insert(buf, 2, "t", 1, NULL);
    buffer_insert(buf, 3, "!", 1, NULL);
    action_group += 1;
    buffer_undo_action_group(buf);
    buffer_get(buf, &data, &data_len);
    ASSERT("ins_group", 0, strncmp(data, "hi", data_len));
}
