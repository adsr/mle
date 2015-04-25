#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "mle.h"

// Run a shell command, optionally feeding stdin, collecting stdout
int util_shell_exec(editor_t* editor, char* cmd, long timeout_s, char* input, size_t input_len, char* opt_shell, char** ret_output, size_t* ret_output_len) {
    int rv;
    int readfd;
    int writefd;
    char* readbuf;
    size_t readbuf_len;
    size_t readbuf_size;
    ssize_t rc;
    ssize_t nbytes;
    fd_set readfds;
    struct timeval timeout;

    // Open cmd
    if (!util_popen2(cmd, opt_shell, &readfd, &writefd)) {
        MLE_RETURN_ERR(editor, "Failed to exec shell cmd: %s", cmd);
    }

    // Close write pipe if no input
    if (input_len < 1) {
        close(writefd);
    }

    // Read-write loop
    readbuf = NULL;
    readbuf_len = 0;
    readbuf_size = 0;
    rv = MLE_OK;
    do {
        // Write to shell cmd if input is remaining
        if (input_len > 0) {
            rc = write(writefd, input, input_len);
            if (rc > 0) {
                input += rc;
                input_len -= rc;
                if (input_len < 1) {
                    close(writefd);
                    writefd = 0;
                }
            } else {
                // write err
                MLE_SET_ERR(editor, "write error: %s", strerror(errno));
                rv = MLE_ERR;
                break;
            }
        }

        // Read shell cmd, timing out after timeout_sec
        timeout.tv_sec = timeout_s;
        timeout.tv_usec = 0;
        FD_ZERO(&readfds);
        FD_SET(readfd, &readfds);
        rc = select(readfd + 1, &readfds, NULL, NULL, &timeout);
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
            if (readbuf_len + 1024 + 1 > readbuf_size) {
                readbuf = realloc(readbuf, readbuf_len + 1024 + 1);
                readbuf_size = readbuf_len + 1024 + 1;
            }
            nbytes = read(readfd, readbuf + readbuf_len, 1024);
            if (nbytes < 0) {
                // read err or EAGAIN/EWOULDBLOCK
                MLE_SET_ERR(editor, "read error: %s", strerror(errno));
                rv = MLE_ERR;
                break;
            } else if (nbytes > 0) {
                // Got data
                readbuf_len += nbytes;
            }
        }
    } while(nbytes > 0);

    // Close pipes
    if (readfd) close(readfd);
    if (writefd) close(writefd);

    *ret_output = readbuf;
    *ret_output_len = readbuf_len;

    return rv;
}


// Like popen, but bidirectional. Returns 1 on success, 0 on failure.
int util_popen2(char* cmd, char* opt_shell, int* ret_fdread, int* ret_fdwrite) {
    FILE* fr;
    pid_t pid;
    int pin[2];
    int pout[2];

    // Set shell
    opt_shell = opt_shell ? opt_shell : "sh";

    // Make pipes
    if (pipe(pin)) return 0;
    if (pipe(pout)) return 0;

    // Fork
    pid = fork();
    if (pid < 0) {
        // Fork failed
        return 0;
    } else if (pid == 0) {
        // Child
        close(pout[0]);
        dup2(pout[1], STDOUT_FILENO);
        close(pout[1]);

        close(pin[1]);
        dup2(pin[0], STDIN_FILENO);
        close(pin[0]);

        execlp(opt_shell, opt_shell, "-c", cmd, NULL);
        exit(EXIT_FAILURE);
    }
    // Parent
    close(pout[1]);
    close(pin[0]);
    fr = fdopen(dup(pout[0]), "r");
    setvbuf(fr, NULL, _IONBF, 0);
    fclose(fr);
    *ret_fdread = pout[0];
    *ret_fdwrite = pin[1];
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
int util_pcre_match(char* subject, char* re) {
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

// Return 1 if a > b, else return 0.
int util_timeval_is_gt(struct timeval* a, struct timeval* b) {
    if (a->tv_sec > b->tv_sec) {
        return 1;
    }
    return a->tv_usec > b->tv_usec ? 1 : 0;
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
