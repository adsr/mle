#include <wchar.h>
#include "mlbuf.h"

// Move self/col forward until col fits on current line
static void _bline_advance_col(bline_t** self, bint_t* col) {
    while (1) {
        MLBUF_BLINE_ENSURE_CHARS(*self);
        if (*col > (*self)->char_count) {
            if ((*self)->next) {
                *col -= (*self)->char_count + 1;
                *self = (*self)->next;
            } else {
                *col = (*self)->char_count;
                break;
            }
        } else {
            break;
        }
    }
}

// Count multi-byte characters and character widths of this line
int bline_count_chars(bline_t* bline) {
    char* c;
    int char_len;
    uint32_t ch;
    int char_w;
    bint_t i;
    int is_tabless_ascii;

    // Unmark dirty
    if (bline->is_chars_dirty) bline->is_chars_dirty = 0;

    // Return early if there is no data
    if (bline->data_len < 1) {
        bline->char_count = 0;
        bline->char_vwidth = 0;
        return MLBUF_OK;
    }

    // Ensure space for chars
    // It should have data_len elements at most
    if (!bline->chars) {
        bline->chars = calloc(bline->data_len, sizeof(bline_char_t));
        bline->chars_cap = bline->data_len;
    } else if (!bline->is_data_slabbed && bline->data_len > bline->chars_cap) {
        bline->chars = recalloc(bline->chars, bline->chars_cap, bline->data_len, sizeof(bline_char_t));
        bline->chars_cap = bline->data_len;
    }

    // Attempt shortcut for lines with all ascii and no tabs
    is_tabless_ascii = 1;
    c = bline->data;
    i = 0;
    while (c < MLBUF_BLINE_DATA_STOP(bline)) {
        if ((*c & 0x80) || *c == '\t') {
            is_tabless_ascii = 0;
            break;
        }
        bline->chars[i].ch = *c;
        bline->chars[i].len = 1;
        bline->chars[i].index = i;
        bline->chars[i].vcol = i;
        bline->chars[i].index_to_vcol = i;
        i++;
        c++;
    }
    bline->char_count = i;
    bline->char_vwidth = i;

    if (!is_tabless_ascii) {
        // We encountered either non-ascii or a tab above, so we have to do a
        // little more work.
        while (c < MLBUF_BLINE_DATA_STOP(bline)) {
            ch = 0;
            char_len = utf8_char_to_unicode(&ch, c, MLBUF_BLINE_DATA_STOP(bline));
            if (ch == '\t') {
                // Special case for tabs
                char_w = bline->buffer->tab_width - (bline->char_vwidth % bline->buffer->tab_width);
            } else {
                char_w = wcwidth(ch);
            }
            // Let null and non-printable chars occupy 1 column
            if (char_w < 1) char_w = 1;
            if (char_len < 1) char_len = 1;
            bline->chars[bline->char_count].ch = ch;
            bline->chars[bline->char_count].len = char_len;
            bline->chars[bline->char_count].index = (bint_t)(c - bline->data);
            bline->chars[bline->char_count].vcol = bline->char_vwidth;
            for (i = 0; i < char_len; i++) if ((c - bline->data) + i < bline->data_len) {
                bline->chars[(c - bline->data) + i].index_to_vcol = bline->char_count;
            }
            bline->char_count += 1;
            bline->char_vwidth += char_w;
            c += char_len;
        }
    }

    return MLBUF_OK;
}

// Insert data on a line
int bline_insert(bline_t* self, bint_t col, char* data, bint_t data_len, bint_t* ret_num_chars) {
    _bline_advance_col(&self, &col);
    return buffer_insert_w_bline(self->buffer, self, col, data, data_len, ret_num_chars);
}

// Delete data from a line
int bline_delete(bline_t* self, bint_t col, bint_t num_chars) {
    _bline_advance_col(&self, &col);
    return buffer_delete_w_bline(self->buffer, self, col, num_chars);
}

// Replace data on a line
int bline_replace(bline_t* self, bint_t col, bint_t num_chars, char* data, bint_t data_len) {
    _bline_advance_col(&self, &col);
    return buffer_replace_w_bline(self->buffer, self, col, num_chars, data, data_len);
}

// Return a col given a byte index
int bline_get_col(bline_t* self, bint_t index, bint_t* ret_col) {
    bint_t col;
    MLBUF_MAKE_GT_EQ0(index);
    MLBUF_BLINE_ENSURE_CHARS(self);
    if (index == 0 || self->char_count == 0) {
        *ret_col = 0;
        return MLBUF_OK;
    } else if (index >= self->data_len) {
        *ret_col = self->char_count;
        return MLBUF_OK;
    }
    for (col = 1; col < self->char_count; col++) {
        if (self->chars[col].index > index) {
            *ret_col = col - 1;
            return MLBUF_OK;
        } else if (self->chars[col].index == index) {
            *ret_col = col;
            return MLBUF_OK;
        }
    }
    *ret_col = self->char_count - (index < self->data_len ? 1 : 0);
    return MLBUF_OK;
}

// Convert a vcol to a col
int bline_get_col_from_vcol(bline_t* bline, bint_t vcol, bint_t* ret_col) {
    bint_t i;
    MLBUF_BLINE_ENSURE_CHARS(bline);
    for (i = 0; i < bline->char_count; i++) {
        if (vcol <= bline->chars[i].vcol) {
            *ret_col = i;
            return MLBUF_OK;
        }
    }
    *ret_col = bline->char_count;
    return MLBUF_OK;
}
