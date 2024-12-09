#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <inttypes.h>
#include <dlfcn.h>
#include "mle.h"
#include "mlbuf.h"

#define concat1(a, b) a ## b
#define concat2(a, b) concat1(a, b)
#define str  concat2(TEST_NAME, _str)
#define test TEST_NAME

extern char *str;
extern void test(buffer_t *buf, mark_t *cur);

// TODO run each test with buffer_set_slabbed + buffer_insert

#define ASSERT(testname, expected, observed) do { \
    if ((expected) == (observed)) { \
        printf("  \x1b[32mOK  \x1b[0m %s\n", (testname)); \
    } else { \
        printf("  \x1b[31mERR \x1b[0m %s expected=%" PRIdPTR " observed=%" PRIdPTR "\n", (testname), (intptr_t)(expected), (intptr_t)(observed)); \
        exit(EXIT_FAILURE); \
    } \
} while (0);
