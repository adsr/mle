#include "test.h"

MAIN("hello\nworld",
    bint_t i;
    srule_t* srule;
    srule = srule_new_single("world", sizeof("world")-1, 0, 1, 2);
    buffer_add_srule(buf, srule);
    buffer_remove_srule(buf, srule);
    for (i = 0; i < buf->first_line->char_count; i++) {
        ASSERT("line1fg", 0, buf->first_line->chars[i].style.fg);
        ASSERT("line1bg", 0, buf->first_line->chars[i].style.bg);
    }
    for (i = 0; i < buf->first_line->next->char_count; i++) {
        ASSERT("line2fg", 0, buf->first_line->next->chars[i].style.fg);
        ASSERT("line2bg", 0, buf->first_line->next->chars[i].style.bg);
    }
    srule_destroy(srule);
)
