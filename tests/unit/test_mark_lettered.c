#include "test.h"

MAIN("hello\nworld",
    mark_t* lett1;
    mark_t* lett2;
    mark_t* lett3;

    mark_clone_w_letter(cur, 'a', &lett1);
    ASSERT("neq1", 1, lett1 != cur ? 1 : 0);
    ASSERT("let", '\0', cur->letter);
    ASSERT("let", 'a', lett1->letter);

    mark_clone_w_letter(cur, 'a', &lett2); // lett1 destroyed
    ASSERT("neq2", 1, lett1 != lett2 ? 1 : 0);

    mark_clone_w_letter(cur, '?', &lett3);
    ASSERT("null", 1, lett3 == NULL ? 1 : 0);

    mark_destroy(lett2);
)
