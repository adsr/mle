#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
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

// Manage async procs, giving priority to user input. Return 1 if drain should
// be called again, else return 0.
int async_proc_drain_all(async_proc_t* aprocs, int* ttyfd) {
    int maxfd;
    fd_set readfds;
    struct timeval timeout;
    struct timeval now;
    async_proc_t* aproc;
    async_proc_t* aproc_tmp;
    char buf[1024 + 1];
    size_t nbytes;
    int rc;
    int is_done;

    // Exit early if no aprocs
    if (!aprocs) return 0;

    // Open ttyfd if not already open
    if (!*ttyfd) {
        if ((*ttyfd = open("/dev/tty", O_RDONLY)) < 0) {
            // TODO error
            return 0;
        }
    }

    // Set timeout to 1s
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    // Add tty to readfds
    FD_ZERO(&readfds);
    FD_SET(*ttyfd, &readfds);

    // Add async procs to readfds
    maxfd = *ttyfd;
    DL_FOREACH(aprocs, aproc) {
        FD_SET(aproc->pipefd, &readfds);
        if (aproc->pipefd > maxfd) maxfd = aproc->pipefd;
    }

    // Perform select
    rc = select(maxfd + 1, &readfds, NULL, NULL, &timeout);
    gettimeofday(&now, NULL);

    if (rc < 0) {
        return 0; // TODO Display errors
    } else if (rc == 0) {
        return 1; // Nothing to read, call again
    }

    if (FD_ISSET(*ttyfd, &readfds)) {
        // Immediately give priority to user input
        return 0;
    } else {
        // Read async procs
        DL_FOREACH_SAFE(aprocs, aproc, aproc_tmp) {
            // Read and invoke callback
            is_done = 0;
            if (FD_ISSET(aproc->pipefd, &readfds)) {
                nbytes = fread(&buf, sizeof(char), 1024, aproc->pipe);
                if (nbytes > 0) {
                    buf[nbytes] = '\0';
                    aproc->callback(aproc, buf, nbytes, ferror(aproc->pipe), feof(aproc->pipe), 0);
                }
                is_done = ferror(aproc->pipe) || feof(aproc->pipe);
            }

            // Close and free if eof, error, or timeout
            // TODO Alert user when timeout occurs
            if (is_done || aproc->is_done || util_timeval_is_gt(&aproc->timeout, &now)) {
                aproc->callback(aproc, NULL, 0, 0, 0, 1);
                async_proc_destroy(aproc); // Calls DL_DELETE
            }
        }
    }

    return 1;
}
