#include "test.h"

MAIN("hi",
    char* reg;
    size_t reg_len;

    buffer_register_set(buf, 'a', "yo", 2);
    buffer_register_get(buf, 'a', 0, &reg, &reg_len);
    ASSERT("set", 0, strncmp(reg, "yo", reg_len))

    buffer_register_prepend(buf, 'a', "g", 1);
    buffer_register_get(buf, 'a', 0, &reg, &reg_len);
    ASSERT("pre", 0, strncmp(reg, "gyo", reg_len))

    buffer_register_append(buf, 'a', "!!!", 3);
    buffer_register_get(buf, 'a', 0, &reg, &reg_len);
    ASSERT("app", 0, strncmp(reg, "gyo!!!", reg_len))

    buffer_register_clear(buf, 'a');
    buffer_register_get(buf, 'a', 0, &reg, &reg_len);
    ASSERT("clr1", 0, reg_len);
    ASSERT("clr2", 1, reg != NULL ? 1 : 0);
)
