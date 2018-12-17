#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include "mle.h"
#include "mlbuf.h"

editor_t _editor;

// TODO run each test with buffer_set_slabbed + buffer_insert

#define MAIN(str, body) \
int main(int argc, char **argv) { \
    buffer_t* buf; \
    mark_t* cur; \
    memset(&_editor, 0, sizeof(editor_t)); \
    setlocale(LC_ALL, ""); \
    buf = buffer_new(); \
    buffer_insert(buf, 0, (char*)str, (bint_t)strlen(str), NULL); \
    cur = buffer_add_mark(buf, NULL, 0); \
    (void)cur; \
    body \
    buffer_destroy(buf); \
    return EXIT_SUCCESS; \
}

#define ASSERT(testname, expected, observed) do { \
    if ((expected) == (observed)) { \
        printf("  \x1b[32mOK \x1b[0m %s\n", (testname)); \
    } else { \
        printf("  \x1b[31mERR\x1b[0m %s expected=%lu observed=%lu\n", (testname), (bint_t)(expected), (bint_t)(observed)); \
        exit(EXIT_FAILURE); \
    } \
} while(0);
