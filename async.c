#include <stdio.h>
#include <stdarg.h>
#include "utlist.h"
#include "mle.h"

// Return a new async_proc_t
async_proc_t* async_proc_new(bview_t* invoker, struct timeval timeout, async_proc_cb_t callback, const char *cmd_fmt, ...) {
    async_proc_t* aproc;
    char* shell_cmd;
    va_list vl;

    // Make shell_cmd
    va_start(vl, cmd_fmt);
    vasprintf(&shell_cmd, cmd_fmt, vl);
    va_end(vl);

    // Make async proc
    aproc = calloc(1, sizeof(async_proc_t));
    aproc->editor = invoker->editor;
    aproc->invoker = invoker;
    aproc->pipe = popen(shell_cmd, "r");
    aproc->timeout = timeout;
    aproc->callback = callback;
    free(shell_cmd);

    DL_APPEND(aproc->editor->async_procs, aproc);
    return aproc;
}

// Destroy an async_proc_t
int async_proc_destroy(async_proc_t* aproc) {
    DL_DELETE(aproc->editor->async_procs, aproc);
    pclose(aproc->pipe);
    free(aproc);
    return MLE_OK;
}
