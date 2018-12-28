#include "test.h"

char *str = "lineA\n\nline2\nline3\n";

void test(buffer_t *buf, mark_t *cur) {
    char *data;
    bint_t data_len;

    buffer_replace(buf, 0, 0, "b", 1);
    buffer_get(buf, &data, &data_len);
    ASSERT("rpl1", 0, strncmp(data, "blineA\n\nline2\nline3\n", data_len));
    ASSERT("r1bc", data_len, buf->byte_count);
    ASSERT("r1lc", 5, buf->line_count);

    buffer_replace(buf, 3, 3, "xe0", 3);
    buffer_get(buf, &data, &data_len);
    ASSERT("rpl2", 0, strncmp(data, "blixe0\n\nline2\nline3\n", data_len));
    ASSERT("r2bc", data_len, buf->byte_count);
    ASSERT("r2lc", 5, buf->line_count);

    buffer_replace(buf, 10, 7, "N", 1);
    buffer_get(buf, &data, &data_len);
    ASSERT("rpl3", 0, strncmp(data, "blixe0\n\nliNe3\n", data_len));
    ASSERT("r3bc", data_len, buf->byte_count);
    ASSERT("r3lc", 4, buf->line_count);

    buffer_replace(buf, 5, 4, "jerk\nstuff", 10);
    buffer_get(buf, &data, &data_len);
    ASSERT("rpl4", 0, strncmp(data, "blixejerk\nstuffiNe3\n", data_len));
    ASSERT("r4bc", data_len, buf->byte_count);
    ASSERT("r4lc", 3, buf->line_count);

    buffer_replace(buf, 9, 99, "X", 1);
    buffer_get(buf, &data, &data_len);
    ASSERT("rpl5", 0, strncmp(data, "blixejerkX", data_len));
    ASSERT("r5bc", data_len, buf->byte_count);
    ASSERT("r5lc", 1, buf->line_count);

    buffer_replace(buf, 5, 0, "y\nb", 3);
    buffer_get(buf, &data, &data_len);
    ASSERT("rpl6", 0, strncmp(data, "blixey\nbjerkX", data_len));
    ASSERT("r6bc", data_len, buf->byte_count);
    ASSERT("r6lc", 2, buf->line_count);

    buffer_replace(buf, 0, 0, "\n", 1);
    buffer_get(buf, &data, &data_len);
    ASSERT("rpl7", 0, strncmp(data, "\nblixey\nbjerkX", data_len));
    ASSERT("r7bc", data_len, buf->byte_count);
    ASSERT("r7lc", 3, buf->line_count);

    buffer_replace(buf, 6, 3, NULL, 0);
    buffer_get(buf, &data, &data_len);
    ASSERT("rpl8", 0, strncmp(data, "\nblixejerkX", data_len));
    ASSERT("r8bc", data_len, buf->byte_count);
    ASSERT("r8lc", 2, buf->line_count);

    buffer_replace(buf, 0, 11, "1\n2\n3\n4\n", 8);
    buffer_get(buf, &data, &data_len);
    ASSERT("rpl9", 0, strncmp(data, "1\n2\n3\n4\n", data_len));
    ASSERT("r9bc", data_len, buf->byte_count);
    ASSERT("r9lc", 5, buf->line_count);

    buffer_replace(buf, 2, 6, "five\nsix\nseven\neight\nnine", 25);
    buffer_get(buf, &data, &data_len);
    ASSERT("rpla", 0, strncmp(data, "1\nfive\nsix\nseven\neight\nnine", data_len));
    ASSERT("rabc", data_len, buf->byte_count);
    ASSERT("ralc", 6, buf->line_count);

    buffer_undo(buf);
    buffer_undo(buf);
    buffer_undo(buf);
    buffer_undo(buf);
    buffer_undo(buf);
    buffer_undo(buf);
    buffer_undo(buf);
    buffer_undo(buf);
    buffer_undo(buf);
    buffer_get(buf, &data, &data_len);
    ASSERT("undo", 0, strncmp(data, "\nblixey\nbjerkX", data_len));
}
