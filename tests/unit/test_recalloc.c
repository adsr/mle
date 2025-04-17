#include "test.h"

typedef struct {
    int a;
    int b;
} thing_t;

char *str = "";

void test(buffer_t *buf, mark_t *cur) {
    int i;
    int is_all_zero;
    thing_t *things;
    int nsize = 1024;

    things = calloc(1, sizeof(thing_t));
    things->a = 1;
    things->b = 2;

    things = recalloc(things, 1, nsize, sizeof(thing_t));
    ASSERT("preserve_a", 1, things->a);
    ASSERT("preserve_b", 2, things->b);

    is_all_zero = 1;
    for (i = 1; i < nsize; i++) {
        if (things[i].a != 0 || things[i].b != 0) {
            is_all_zero = 0;
            break;
        }
    }
    ASSERT("zerod", 1, is_all_zero);

    free(things);
}
