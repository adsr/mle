#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <inttypes.h>
#include "mle.h"
#include "mlbuf.h"

editor_t _editor;
extern char *str;
void test(buffer_t *buf, mark_t *cur);

// TODO run each test with buffer_set_slabbed + buffer_insert

int main(int argc, char **argv) {
    buffer_t *buf;
    mark_t *cur;
    memset(&_editor, 0, sizeof(editor_t));
    setlocale(LC_ALL, "");
    buf = buffer_new();
    buffer_insert(buf, 0, (char*)str, (bint_t)strlen(str), NULL);
    cur = buffer_add_mark(buf, NULL, 0);
    test(buf, cur);
    buffer_destroy(buf);
    return EXIT_SUCCESS;
}

#define ASSERT(testname, expected, observed) do { \
    if ((expected) == (observed)) { \
        printf("  \x1b[32mOK\x1b[0m %s\n", (testname)); \
    } else { \
        printf("  \x1b[31mERR\x1b[0m %s expected=%" PRIdPTR " observed=%" PRIdPTR "\n", (testname), (intptr_t)(expected), (intptr_t)(observed)); \
        exit(EXIT_FAILURE); \
    } \
} while(0);
