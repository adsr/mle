#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include "mle.h"

// Run a shell command, optionally feeding stdin, collecting stdout
// Specify timeout_s=-1 for no timeout
int util_shell_exec(editor_t* editor, char* cmd, long timeout_s, char* input, size_t input_len, int setsid, char* opt_shell, char** optret_output, size_t* optret_output_len) {
    // TODO clean this crap up
    int rv;
    int do_read;
    int do_write;
    int readfd;
    int writefd;
    ssize_t rc;
    ssize_t nbytes;
    fd_set readfds;
    struct timeval timeout;
    struct timeval* timeoutptr;
    pid_t pid;
    str_t readbuf = {0};

    do_read = optret_output != NULL ? 1 : 0;
    do_write = input && input_len > 0 ? 1 : 0;
    readfd = -1;
    writefd = -1;
    pid = -1;
    readbuf.inc = -2; // double capacity on each allocation
    rv = MLE_OK;
    nbytes = 0;
    if (do_read) {
        *optret_output = NULL;
        *optret_output_len = 0;
    }

    // Open cmd
    if (!util_popen2(
        cmd,
        setsid,
        opt_shell,
        do_read ? &readfd : NULL,
        do_write ? &writefd : NULL,
        &pid
    )) {
        MLE_RETURN_ERR(editor, "Failed to exec shell cmd: %s", cmd);
    }

    // Read-write loop
    do {
        // Write to shell cmd if input is remaining
        if (do_write && writefd >= 0) {
            rc = write(writefd, input, input_len);
            if (rc > 0) {
                input += rc;
                input_len -= rc;
                if (input_len < 1) {
                    close(writefd);
                    writefd = -1;
                }
            } else {
                // write err
                MLE_SET_ERR(editor, "write error: %s", strerror(errno));
                rv = MLE_ERR;
                break;
            }
        }

        // Read shell cmd, timing out after timeout_sec
        if (do_read) {
            if (timeout_s >= 0) {
                timeout.tv_sec = timeout_s;
                timeout.tv_usec = 0;
                timeoutptr = &timeout;
            } else {
                timeoutptr = NULL;
            }
            FD_ZERO(&readfds);
            FD_SET(readfd, &readfds);
            rc = select(readfd + 1, &readfds, NULL, NULL, timeoutptr);
            if (rc < 0) {
                // Err on select
                MLE_SET_ERR(editor, "select error: %s", strerror(errno));
                rv = MLE_ERR;
                break;
            } else if (rc == 0) {
                // Timed out
                rv = MLE_ERR;
                break;
            } else {
                // Read a kilobyte of data
                str_ensure_cap(&readbuf, readbuf.len + 1024);
                nbytes = read(readfd, readbuf.data + readbuf.len, 1024);
                if (nbytes < 0) {
                    // read err or EAGAIN/EWOULDBLOCK
                    MLE_SET_ERR(editor, "read error: %s", strerror(errno));
                    rv = MLE_ERR;
                    break;
                } else if (nbytes > 0) {
                    // Got data
                    readbuf.len += nbytes;
                }
            }
        }
    } while(nbytes > 0); // until EOF

    // Close pipes and reap child proc
    if (readfd >= 0) close(readfd);
    if (writefd >= 0) close(writefd);
    waitpid(pid, NULL, do_read ? WNOHANG : 0);

    if (do_read) {
        *optret_output = readbuf.data;
        *optret_output_len = readbuf.len;
    }

    return rv;
}

// Like popen, but more control over pipes. Returns 1 on success, 0 on failure.
int util_popen2(char* cmd, int do_setsid, char* opt_shell, int* optret_fdread, int* optret_fdwrite, pid_t* optret_pid) {
    pid_t pid;
    int do_read;
    int do_write;
    int pout[2];
    int pin[2];

    // Set r/w
    do_read = optret_fdread != NULL ? 1 : 0;
    do_write = optret_fdwrite != NULL ? 1 : 0;

    // Set shell
    opt_shell = opt_shell ? opt_shell : "sh";

    // Make pipes
    if (do_read) if (pipe(pout)) return 0;
    if (do_write) if (pipe(pin)) return 0;

    // Fork
    pid = fork();
    if (pid < 0) {
        // Fork failed
        return 0;
    } else if (pid == 0) {
        // Child
        if (do_read) {
            close(pout[0]);
            dup2(pout[1], STDOUT_FILENO);
            close(pout[1]);
        }
        if (do_write) {
            close(pin[1]);
            dup2(pin[0], STDIN_FILENO);
            close(pin[0]);
        }
        if (do_setsid) setsid();
        execlp(opt_shell, opt_shell, "-c", cmd, NULL);
        exit(EXIT_FAILURE);
    }
    // Parent
    if (do_read) {
        close(pout[1]);
        *optret_fdread = pout[0];
    }
    if (do_write) {
        close(pin[0]);
        *optret_fdwrite = pin[1];
    }
    if (optret_pid) *optret_pid = pid;
    return 1;
}

// Return paired bracket if ch is a bracket, else return 0
int util_get_bracket_pair(uint32_t ch, int* optret_is_closing) {
    switch (ch) {
        case '[': if (optret_is_closing) *optret_is_closing = 0; return ']';
        case '(': if (optret_is_closing) *optret_is_closing = 0; return ')';
        case '{': if (optret_is_closing) *optret_is_closing = 0; return '}';
        case ']': if (optret_is_closing) *optret_is_closing = 1; return '[';
        case ')': if (optret_is_closing) *optret_is_closing = 1; return '(';
        case '}': if (optret_is_closing) *optret_is_closing = 1; return '{';
        default: return 0;
    }
    return 0;
}

// Return 1 if path is file
int util_is_file(char* path, char* opt_mode, FILE** optret_file) {
    struct stat sb;
    if (stat(path, &sb) != 0 || !S_ISREG(sb.st_mode)) return 0;
    if (opt_mode && optret_file) {
        *optret_file = fopen(path, opt_mode);
        if (!*optret_file) return 0;
    }
    return 1;
}

// Return 1 if path is dir
int util_is_dir(char* path) {
    struct stat sb;
    if (stat(path, &sb) != 0 || !S_ISDIR(sb.st_mode)) return 0;
    return 1;
}

// Return 1 if re matches subject
int util_pcre_match(char* re, char* subject) {
    int rc;
    pcre* cre;
    const char *error;
    int erroffset;
    cre = pcre_compile((const char*)re, PCRE_NO_AUTO_CAPTURE | PCRE_CASELESS, &error, &erroffset, NULL);
    if (!cre) return 0;
    rc = pcre_exec(cre, NULL, subject, strlen(subject), 0, 0, NULL, 0);
    pcre_free(cre);
    return rc >= 0 ? 1 : 0;
}

// Perform a regex replace with back-references. Return number of replacements
// made. If regex is invalid, `ret_result` is set to NULL, `ret_result_len` is
// set to 0 and 0 is returned.
int util_pcre_replace(char* re, char* subj, char* repl, char** ret_result, int* ret_result_len) {
    int rc;
    pcre* cre;
    const char *error;
    int erroffset;
    int subj_offset;
    int subj_offset_z;
    int subj_len;
    int subj_look_offset;
    int ovector[30];
    int num_repls;
    int got_match = 0;
    str_t result = {0};

    *ret_result = NULL;
    *ret_result_len = 0;

    // Compile regex
    cre = pcre_compile((const char*)re, PCRE_CASELESS, &error, &erroffset, NULL);
    if (!cre) return 0;

    // Start match-replace loop
    num_repls = 0;
    subj_len = strlen(subj);
    subj_offset = 0;
    subj_offset_z = 0;
    subj_look_offset = 0;
    while (subj_offset < subj_len) {
        // Find match
        rc = pcre_exec(cre, NULL, subj, subj_len, subj_look_offset, 0, ovector, 30);
        if (rc < 0 || ovector[0] < 0) {
            got_match = 0;
            subj_offset_z = subj_len;
        } else {
            got_match = 1;
            subj_offset_z = ovector[0];
        }

        // Append part before match
        str_append_stop(&result, subj + subj_offset, subj + subj_offset_z);
        subj_offset = ovector[1];
        subj_look_offset = subj_offset + 1;

        // Break if no match
        if (!got_match) break;

        // Append replacements with backrefs
        str_append_replace_with_backrefs(&result, subj, repl, rc, ovector, 30);

        // Increment num_repls
        num_repls += 1;
    }

    // Free regex
    pcre_free(cre);

    // Return result
    *ret_result = result.data ? result.data : strdup("");
    *ret_result_len = result.len;

    // Return number of replacements
    return num_repls;
}

// Return 1 if a > b, else return 0.
int util_timeval_is_gt(struct timeval* a, struct timeval* b) {
    if (a->tv_sec > b->tv_sec) {
        return 1;
    } else if (a->tv_sec == b->tv_sec) {
        return a->tv_usec > b->tv_usec ? 1 : 0;
    }
    return 0;
}

// Ported from php_escape_shell_arg
// https://github.com/php/php-src/blob/master/ext/standard/exec.c
char* util_escape_shell_arg(char* str, int l) {
    int x, y = 0;
    char *cmd;

    cmd = malloc(4 * l + 3); // worst case

    cmd[y++] = '\'';

    for (x = 0; x < l; x++) {
        int mb_len = tb_utf8_char_length(*(str + x));

        // skip non-valid multibyte characters
        if (mb_len < 0) {
            continue;
        } else if (mb_len > 1) {
            memcpy(cmd + y, str + x, mb_len);
            y += mb_len;
            x += mb_len - 1;
            continue;
        }

        switch (str[x]) {
        case '\'':
            cmd[y++] = '\'';
            cmd[y++] = '\\';
            cmd[y++] = '\'';
            // fall-through
        default:
            cmd[y++] = str[x];
        }
    }
    cmd[y++] = '\'';
    cmd[y] = '\0';

    return cmd;
}

// Adapted from termbox src/demo/keyboard.c
int tb_print(int x, int y, uint16_t fg, uint16_t bg, char *str) {
    uint32_t uni;
    int c;
    c = 0;
    while (*str) {
        str += utf8_char_to_unicode(&uni, str, NULL);
        tb_change_cell(x, y, uni, fg, bg);
        x++;
        c++;
    }
    return c;
}

// Adapted from termbox src/demo/keyboard.c
int tb_printf(bview_rect_t rect, int x, int y, uint16_t fg, uint16_t bg, const char *fmt, ...) {
    char buf[4096];
    va_list vl;
    va_start(vl, fmt);
    vsnprintf(buf, sizeof(buf), fmt, vl);
    va_end(vl);
    return tb_print(rect.x + x, rect.y + y, fg ? fg : rect.fg, bg ? bg : rect.bg, buf);
}

// Like tb_printf, but accepts @fg,bg; attributes inside the string. To print
// a literal '@', use '@@' in the format string. Specify fg or bg of 0 to
// reset that attribute.
int tb_printf_attr(bview_rect_t rect, int x, int y, const char *fmt, ...) {
    char bufo[4096];
    char* buf;
    int fg;
    int bg;
    int tfg;
    int tbg;
    int c;
    uint32_t uni;

    va_list vl;
    va_start(vl, fmt);
    vsnprintf(bufo, sizeof(bufo), fmt, vl);
    va_end(vl);

    fg = rect.fg;
    bg = rect.bg;
    x = rect.x + x;
    y = rect.y + y;

    c = 0;
    buf = bufo;
    while (*buf) {
        buf += utf8_char_to_unicode(&uni, buf, NULL);
        if (uni == '@') {
            if (!*buf) break;
            utf8_char_to_unicode(&uni, buf, NULL);
            if (uni != '@') {
                tfg = strtol(buf, &buf, 10);
                if (!*buf) break;
                utf8_char_to_unicode(&uni, buf, NULL);
                if (uni == ',') {
                    buf++;
                    if (!*buf) break;
                    tbg = strtol(buf, &buf, 10);
                    fg = tfg <= 0 ? rect.fg : tfg;
                    bg = tbg <= 0 ? rect.bg : tbg;
                    if (!*buf) break;
                    utf8_char_to_unicode(&uni, buf, NULL);
                    if (uni == ';') buf++;
                    continue;
                }
            }
        }
        tb_change_cell(x, y, uni, fg, bg);
        x++;
        c++;
    }
    return c;
}
