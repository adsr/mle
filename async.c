#include <stdio.h>
#include <stdarg.h>
#include "utlist.h"
#include "mle.h"

// Return a new async_proc_t
async_proc_t* async_proc_new(bview_t* invoker, int timeout_sec, int timeout_usec, async_proc_cb_t callback, char* shell_cmd) {
    async_proc_t* aproc;

    // Make async proc
    aproc = calloc(1, sizeof(async_proc_t));
    async_proc_set_invoker(aproc, invoker);
    aproc->pipe = popen(shell_cmd, "r");
    setvbuf(aproc->pipe, NULL, _IONBF, 0);
    aproc->pipefd = fileno(aproc->pipe);
    aproc->timeout.tv_sec = timeout_sec;
    aproc->timeout.tv_usec = timeout_usec;
    aproc->callback = callback;

    DL_APPEND(aproc->editor->async_procs, aproc);
    return aproc;
}

// Set invoker
int async_proc_set_invoker(async_proc_t* aproc, bview_t* invoker) {
    if (aproc->invoker) {
        aproc->invoker->async_proc = NULL;
    }
    aproc->invoker = invoker;
    aproc->editor = invoker->editor;
    invoker->async_proc = aproc;
    return MLE_OK;
}

// Destroy an async_proc_t
int async_proc_destroy(async_proc_t* aproc) {
    DL_DELETE(aproc->editor->async_procs, aproc);
    pclose(aproc->pipe);
    if (aproc->invoker && aproc->invoker->async_proc == aproc) {
        aproc->invoker->async_proc = NULL;
    }
    free(aproc);
    return MLE_OK;
}
