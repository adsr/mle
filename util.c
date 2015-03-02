#include <stdarg.h>
#include "mle.h"

int util_file_exists(char* path, int path_len) {
    // TODO util_file_exists
    return 0;
}

void tb_print(int x, int y, uint16_t fg, uint16_t bg, char *str) {
    // Adapted from termbox src/demo/keyboard.c
    uint32_t uni;
    while (*str) {
        str += tb_utf8_char_to_unicode(&uni, str);
        tb_change_cell(x, y, uni, fg, bg);
        x++;
    }
}

void tb_printf(bview_rect_t rect, int x, int y, uint16_t fg, uint16_t bg, const char *fmt, ...) {
    // Adapted from termbox src/demo/keyboard.c
    char buf[4096];
    va_list vl;
    va_start(vl, fmt);
    vsnprintf(buf, sizeof(buf), fmt, vl);
    va_end(vl);
    tb_print(rect.x + x, rect.y + y, rect.fg | fg, rect.bg | bg, buf);
}
