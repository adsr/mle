#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "mle.h"

// Return 1 if path is file
int util_file_exists(char* path, char* opt_mode, FILE** optret_file) {
    struct stat sb;
    if (stat(path, &sb) != 0 || !S_ISREG(sb.st_mode)) return 0;
    if (opt_mode && optret_file) {
        *optret_file = fopen(path, opt_mode);
        if (!*optret_file) return 0;
    }
    return 1;
}

// Return 1 if path is dir
int util_dir_exists(char* path) {
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
    cre = pcre_compile((const char*)re, PCRE_NO_AUTO_CAPTURE, &error, &erroffset, NULL);
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
void tb_print(int x, int y, uint16_t fg, uint16_t bg, char *str) {
    uint32_t uni;
    while (*str) {
        str += utf8_char_to_unicode(&uni, str, NULL);
        tb_change_cell(x, y, uni, fg, bg);
        x++;
    }
}

// Adapted from termbox src/demo/keyboard.c
void tb_printf(bview_rect_t rect, int x, int y, uint16_t fg, uint16_t bg, const char *fmt, ...) {
    char buf[4096];
    va_list vl;
    va_start(vl, fmt);
    vsnprintf(buf, sizeof(buf), fmt, vl);
    va_end(vl);
    tb_print(rect.x + x, rect.y + y, rect.fg | fg, rect.bg | bg, buf);
}
