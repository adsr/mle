#include "test.h"

MAIN("hello\nworld",
    char *data;
    bint_t data_len;
    buffer_set(buf, "goodbye\nvoid", 12);
    buffer_get(buf, &data, &data_len);
    ASSERT("set", 0, strncmp("goodbye\nvoid", data, data_len));
)
