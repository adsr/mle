#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <inttypes.h>
#include <dlfcn.h>
#include "mle.h"
#include "mlbuf.h"

editor_t _editor; // satisfies extern in mle.h

int main(int argc, char **argv) {
    buffer_t *buf;
    mark_t *cur;
    void *self;
    void (*symfn)(buffer_t *, mark_t *);
    char **symstr;
    char symbuf[1024];

    if (argc < 2) return EXIT_FAILURE;

    self = dlopen(NULL, RTLD_NOW);
    if (!self) return EXIT_FAILURE;


    *(void **)(&symfn) = dlsym(self, argv[1]);
    sprintf(symbuf, "%s_str", argv[1]);
    symstr = (char**)dlsym(self, symbuf);
    if (!symfn || !symstr) return EXIT_FAILURE;

    setlocale(LC_ALL, "");

    memset(&_editor, 0, sizeof(editor_t));
    buf = buffer_new();
    buffer_insert(buf, 0, *symstr, (bint_t)strlen(*symstr), NULL);
    cur = buffer_add_mark(buf, NULL, 0);

    symfn(buf, cur);

    buffer_destroy(buf);

    dlclose(self);

    return EXIT_SUCCESS;
}
