#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pcre2.h>
#include "mlbuf.h"

static int mark_block_x_between(mark_t *self, mark_t *other, int del, char **optret_str, bint_t *optret_str_len);
typedef char* (*mark_find_match_fn)(char *haystack, bint_t haystack_len, bint_t look_offset, bint_t max_offset, void *u1, void *u2, bint_t *ret_needle_len);
static int mark_find_match(mark_t *self, mark_find_match_fn matchfn, void *u1, void *u2, int reverse, bline_t **ret_line, bint_t *ret_col, bint_t *ret_num_chars);
static int mark_find_re(mark_t *self, char *re, bint_t re_len, int reverse, bline_t **ret_line, bint_t *ret_col, bint_t *ret_num_chars);
static char *mark_find_match_prev(char *haystack, bint_t haystack_len, bint_t look_offset, bint_t max_offset, mark_find_match_fn matchfn, void *u1, void *u2);
static char *mark_find_next_str_matchfn(char *haystack, bint_t haystack_len, bint_t look_offset, bint_t max_offset, void *needle, void *needle_len, bint_t *ret_needle_len);
static char *mark_find_prev_str_matchfn(char *haystack, bint_t haystack_len, bint_t look_offset, bint_t max_offset, void *needle, void *needle_len, bint_t *ret_needle_len);
static char *mark_find_next_cre_matchfn(char *haystack, bint_t haystack_len, bint_t look_offset, bint_t max_offset, void *cre, void *unused, bint_t *ret_needle_len);
static char *mark_find_prev_cre_matchfn(char *haystack, bint_t haystack_len, bint_t look_offset, bint_t max_offset, void *cre, void *unused, bint_t *ret_needle_len);

pcre2_match_data *pcre2_md = NULL;
static PCRE2_SIZE *pcre_ovector = NULL;
static int pcre_ovector_size = 0;
static int *pcre_rc;
static char bracket_pairs[8] = {
    '[', ']',
    '(', ')',
    '{', '}'
};

// Return a clone (same position) of an existing mark
int mark_clone(mark_t *self, mark_t **ret_mark) {
    *ret_mark = buffer_add_mark(self->bline->buffer, self->bline, self->col);
    return MLBUF_OK;
}

// Return a lettered clone (same position) of an existing mark
int mark_clone_w_letter(mark_t *self, char letter, mark_t **ret_mark) {
    *ret_mark = buffer_add_mark_ex(self->bline->buffer, letter, self->bline, self->col);
    return MLBUF_OK;
}

// Insert data before mark
int mark_insert_before(mark_t *self, char *data, bint_t data_len) {
    return bline_insert(self->bline, self->col, data, data_len, NULL);
}

// Insert data after mark
int mark_insert_after(mark_t *self, char *data, bint_t data_len) {
    int rc;
    bint_t num_chars;
    if ((rc = bline_insert(self->bline, self->col, data, data_len, &num_chars)) == MLBUF_OK) {
        rc = mark_move_by(self, -1 * num_chars);
    }
    return rc;
}

// Delete data after mark
int mark_delete_after(mark_t *self, bint_t num_chars) {
    return bline_delete(self->bline, self->col, num_chars);
}

// Delete data before mark
int mark_delete_before(mark_t *self, bint_t num_chars) {
    int rc;
    if ((rc = mark_move_by(self, -1 * num_chars)) == MLBUF_OK) {
        rc = mark_delete_after(self, num_chars);
    }
    return rc;
}

// Replace data
int mark_replace(mark_t *self, bint_t num_chars, char *data, bint_t data_len) {
    return bline_replace(self->bline, self->col, num_chars, data, data_len);
}

// Replace data between marks
int mark_replace_between(mark_t *self, mark_t *other, char *data, bint_t data_len) {
    bint_t offset_a;
    bint_t offset_b;
    // TODO More efficient buffer_replace_w_bline_and_end
    buffer_get_offset(self->bline->buffer, self->bline, self->col, &offset_a);
    buffer_get_offset(other->bline->buffer, other->bline, other->col, &offset_b);
    if (offset_a < offset_b) {
        return buffer_replace(self->bline->buffer, offset_a, offset_b - offset_a, data, data_len);
    }
    return buffer_replace(self->bline->buffer, offset_b, offset_a - offset_b, data, data_len);
}

// Move mark to bline:col
int mark_move_to_w_bline(mark_t *self, bline_t *bline, bint_t col) {
    _mark_mark_move_inner(self, bline, col, 1);
    return MLBUF_OK;
}

// Move mark to line_index:col
int mark_move_to(mark_t *self, bint_t line_index, bint_t col) {
    bline_t *bline;
    buffer_get_bline_w_hint(self->bline->buffer, line_index, self->bline, &bline);
    _mark_mark_move_inner(self, bline, col, 1);
    return MLBUF_OK;
}

// Move mark by a character delta
int mark_move_by(mark_t *self, bint_t char_delta) {
    bint_t offset;
    buffer_get_offset(self->bline->buffer, self->bline, self->col, &offset);
    return mark_move_offset(self, offset + char_delta);
}

// Get mark offset
int mark_get_offset(mark_t *self, bint_t *ret_offset) {
    return buffer_get_offset(self->bline->buffer, self->bline, self->col, ret_offset);
}

// Move mark by line delta
int mark_move_vert(mark_t *self, bint_t line_delta) {
    bline_t *cur_line;
    bline_t *tmp_line;
    cur_line = self->bline;
    while (line_delta != 0) {
        tmp_line = line_delta > 0 ? cur_line->next : cur_line->prev;
        if (!tmp_line) {
            break;
        }
        cur_line = tmp_line;
        line_delta = line_delta + (line_delta > 0 ? -1 : 1);
    }
    if (cur_line == self->bline) {
        return MLBUF_OK;
    }
    _mark_mark_move_inner(self, cur_line, self->target_col, 0);
    return MLBUF_OK;
}

// Move mark to beginning of line
int mark_move_bol(mark_t *self) {
    _mark_mark_move_inner(self, self->bline, 0, 1);
    return MLBUF_OK;
}

// Move mark to end of line
int mark_move_eol(mark_t *self) {
    MLBUF_BLINE_ENSURE_CHARS(self->bline);
    _mark_mark_move_inner(self, self->bline, self->bline->char_count, 1);
    return MLBUF_OK;
}

// Move mark to a column on the current line
int mark_move_col(mark_t *self, bint_t col) {
    _mark_mark_move_inner(self, self->bline, col, 1);
    return MLBUF_OK;
}

// Move mark to beginning of buffer
int mark_move_beginning(mark_t *self) {
    _mark_mark_move_inner(self, self->bline->buffer->first_line, 0, 1);
    return MLBUF_OK;
}

// Move mark to end of buffer
int mark_move_end(mark_t *self) {
    MLBUF_BLINE_ENSURE_CHARS(self->bline->buffer->last_line);
    _mark_mark_move_inner(self, self->bline->buffer->last_line, self->bline->buffer->last_line->char_count, 1);
    return MLBUF_OK;
}

// Move mark to a particular offset
int mark_move_offset(mark_t *self, bint_t offset) {
    bline_t *dest_line;
    bint_t dest_col;
    buffer_get_bline_col(self->bline->buffer, offset, &dest_line, &dest_col);
    _mark_mark_move_inner(self, dest_line, dest_col, 1);
    return MLBUF_OK;
}

// Find next occurrence of string from mark
int mark_find_next_str(mark_t *self, char *str, bint_t str_len, bline_t **ret_line, bint_t *ret_col, bint_t *ret_num_chars) {
    return mark_find_match(self, mark_find_next_str_matchfn, (void*)str, (void*)&str_len, 0, ret_line, ret_col, ret_num_chars);
}

// Find prev occurrence of string from mark
int mark_find_prev_str(mark_t *self, char *str, bint_t str_len, bline_t **ret_line, bint_t *ret_col, bint_t *ret_num_chars) {
    return mark_find_match(self, mark_find_prev_str_matchfn, (void*)str, (void*)&str_len, 1, ret_line, ret_col, ret_num_chars);
}

// Find next occurence of regex from mark
int mark_find_next_cre(mark_t *self, pcre2_code *cre, bline_t **ret_line, bint_t *ret_col, bint_t *ret_num_chars) {
    return mark_find_match(self, mark_find_next_cre_matchfn, (void*)cre, NULL, 0, ret_line, ret_col, ret_num_chars);
}

// Find prev occurence of regex from mark
int mark_find_prev_cre(mark_t *self, pcre2_code *cre, bline_t **ret_line, bint_t *ret_col, bint_t *ret_num_chars) {
    return mark_find_match(self, mark_find_prev_cre_matchfn, (void*)cre, NULL, 1, ret_line, ret_col, ret_num_chars);
}

// Find next occurence of uncompiled regex str from mark
int mark_find_next_re(mark_t *self, char *re, bint_t re_len, bline_t **ret_line, bint_t *ret_col, bint_t *ret_num_chars) {
    return mark_find_re(self, re, re_len, 0, ret_line, ret_col, ret_num_chars);
}

// Find prev occurence of uncompiled regex str from mark
int mark_find_prev_re(mark_t *self, char *re, bint_t re_len, bline_t **ret_line, bint_t *ret_col, bint_t *ret_num_chars) {
    return mark_find_re(self, re, re_len, 1, ret_line, ret_col, ret_num_chars);
}

// Return 1 if self is before other, otherwise return 0
int mark_is_lt(mark_t *self, mark_t *other) {
    if (self->bline->line_index == other->bline->line_index) {
        return self->col < other->col ? 1 : 0;
    } else if (self->bline->line_index < other->bline->line_index) {
        return 1;
    }
    return 0;
}

// Return 1 if self is past other, otherwise return 0
int mark_is_gt(mark_t *self, mark_t *other) {
    if (self->bline->line_index == other->bline->line_index) {
        return self->col > other->col ? 1 : 0;
    } else if (self->bline->line_index > other->bline->line_index) {
        return 1;
    }
    return 0;
}

// Return 1 if self is at same position as other, otherwise return 0
int mark_is_eq(mark_t *self, mark_t *other) {
    if (self->bline->line_index == other->bline->line_index) {
        return self->col == other->col ? 1 : 0;
    }
    return 0;
}

// Return 1 if self >= other
int mark_is_gte(mark_t *self, mark_t *other) {
    return !mark_is_lt(self, other);
}

// Return 1 if self <= other
int mark_is_lte(mark_t *self, mark_t *other) {
    return !mark_is_gt(self, other);
}

// Return 1 if self is between a and b
int mark_is_between(mark_t *self, mark_t *ma, mark_t *mb) {
    mark_t *a, *b;
    mark_cmp(ma, mb, &a, &b);
    return mark_is_gte(self, a) && mark_is_lt(self, b) ? 1 : 0;
}

// Find top-level bracket to the left examining no more than max_chars
int mark_find_bracket_top(mark_t *self, bint_t max_chars, bline_t **ret_line, bint_t *ret_col, bint_t *ret_brkt) {
    bline_t *cur_line;
    bint_t col;
    int *stacks;
    int found;
    int i;
    int i_left;
    cur_line = self->bline;
    col = self->col;
    stacks = calloc(128, sizeof(int));
    found = 0;
    while (!found && max_chars > 0 && cur_line) {
        MLBUF_BLINE_ENSURE_CHARS(cur_line);
        col -= 1;
        if (col < 0) {
            cur_line = cur_line->prev;
            if (cur_line) col = cur_line->char_count;
            max_chars -= 1;
            continue;
        }
        for (i = 0; i < 8; i++) {
            i_left = (i % 2 == 0 ? i : i - 1);
            if (cur_line->chars[col].ch == (uint32_t)bracket_pairs[i]) {
                stacks[(int)bracket_pairs[i_left]] += (i % 2 == 0 ? -1 : 1);
                if (stacks[(int)bracket_pairs[i_left]] <= -1) {
                    *ret_line = cur_line;
                    *ret_col = col;
                    *ret_brkt = bracket_pairs[i];
                    found = 1;
                }
                break;
            }
        }
    }
    free(stacks);
    return found ? MLBUF_OK : MLBUF_ERR;
}

// Find the matching bracket character under the mark, examining no more than
// max_chars.
int mark_find_bracket_pair(mark_t *self, bint_t max_chars, bline_t **ret_line, bint_t *ret_col, bint_t *ret_brkt) {
    char brkt;
    char targ;
    char cur;
    int dir;
    int i;
    int nest;
    bint_t col;
    bint_t nchars;
    bline_t *cur_line;
    MLBUF_BLINE_ENSURE_CHARS(self->bline);

    // If we're at eol, there's nothing to match
    if (self->col >= self->bline->char_count) {
        return MLBUF_ERR;
    }
    // Set brkt to char under mark
    brkt = *(self->bline->data + self->bline->chars[self->col].index);
    // Find targ matching bracket char
    targ = 0;
    for (i = 0; i < 8; i++) {
        if (bracket_pairs[i] == brkt) {
            if (i % 2 == 0) {
                targ = bracket_pairs[i + 1];
                dir = 1;
            } else {
                targ = bracket_pairs[i - 1];
                dir = -1;
            }
            break;
        }
    }
    // If targ is not set, brkt was not a bracket char
    if (!targ) {
        return MLBUF_ERR;
    }
    // Now look for targ, keeping track of nesting
    // Break if we look at more than max_chars
    nest = -1;
    cur_line = self->bline;
    col = self->col;
    nchars = 0;
    while (cur_line) {
        MLBUF_BLINE_ENSURE_CHARS(cur_line);
        for (; col >= 0 && col < cur_line->char_count; col += dir) {
            cur = *(cur_line->data + cur_line->chars[col].index);
            if (cur == targ) {
                if (nest == 0) {
                    // Match!
                    *ret_line = cur_line;
                    *ret_col = col;
                    *ret_brkt = targ;
                    return MLBUF_OK;
                } else {
                    nest -= 1;
                }
            } else if (cur == brkt) {
                nest += 1;
            }
            nchars += 1;
            if (nchars >= max_chars) {
                return MLBUF_ERR;
            }
        }
        if (dir > 0) {
            cur_line = cur_line->next;
            if (cur_line) col = 0;
        } else {
            cur_line = cur_line->prev;
            if (cur_line) col = MLBUF_MAX(1, cur_line->char_count) - 1;
        }
    }
    // If we got here, targ was not found, or nesting was off
    return MLBUF_ERR;
}

// Delete data between self and other
int mark_delete_between(mark_t *self, mark_t *other) {
    bint_t offset_a;
    bint_t offset_b;
    // TODO More efficient buffer_replace_w_bline_and_end
    buffer_get_offset(self->bline->buffer, self->bline, self->col, &offset_a);
    buffer_get_offset(other->bline->buffer, other->bline, other->col, &offset_b);
    if (offset_a == offset_b) {
        return MLBUF_OK;
    } else if (offset_a > offset_b) {
        return buffer_delete(self->bline->buffer, offset_b, offset_a - offset_b);
    }
    return buffer_delete(self->bline->buffer, offset_a, offset_b - offset_a);
}

// Return data between self and other
int mark_get_between(mark_t *self, mark_t *other, char **ret_str, bint_t *ret_str_len) {
    bint_t ig;
    mark_t *a, *b;
    if (mark_cmp(self, other, &a, &b) == 0) {
        *ret_str = strdup("");
        *ret_str_len = 0;
        return MLBUF_OK;
    }
    return buffer_substr(
        a->bline->buffer,
        a->bline, a->col,
        b->bline, b->col,
        ret_str, ret_str_len, &ig
    );
}

// Return num chars between two marks
int mark_get_nchars_between(mark_t *self, mark_t *other, bint_t *ret_nchars) {
    mark_t *a, *b;
    bline_t *bline;
    bint_t col, nchars;
    if (mark_cmp(self, other, &a, &b) == 0) {
        *ret_nchars = 0;
        return MLBUF_OK;
    }
    bline = a->bline;
    col = a->col;
    nchars = 0;
    while (bline != b->bline) {
        MLBUF_BLINE_ENSURE_CHARS(bline);
        nchars += (bline->char_count - col) + 1;
        bline = bline->next;
        col = 0;
        if (!bline) {
            *ret_nchars = 0;
            return MLBUF_ERR;
        }
    }
    nchars += (b->col - col);
    *ret_nchars = nchars;
    return MLBUF_OK;
}

// Move self to other
int mark_join(mark_t *self, mark_t *other) {
    _mark_mark_move_inner(self, other->bline, other->col, 1);
    return MLBUF_OK;
}

// Swap positions of self and other
int mark_swap(mark_t *self, mark_t *other) {
    mark_t tmp_mark;
    tmp_mark.bline = other->bline;
    tmp_mark.col = other->col;
    _mark_mark_move_inner(other, self->bline, self->col, 1);
    _mark_mark_move_inner(self, tmp_mark.bline, tmp_mark.col, 1);
    return MLBUF_OK;
}

// Return 1 if mark is at eol, else return 0
int mark_is_at_eol(mark_t *self) {
    MLBUF_BLINE_ENSURE_CHARS(self->bline);
    return self->col >= self->bline->char_count ? 1 : 0;
}

// Return 1 if mark is at bol, else return 0
int mark_is_at_bol(mark_t *self) {
    return self->col <= 0;
}

// Destroy a mark
int mark_destroy(mark_t *self) {
    return buffer_destroy_mark(self->bline->buffer, self);
}

#define MLBUF_MARK_IMPLEMENT_MOVE_VIA_FIND_EX(mark, findfn, ...) do { \
    int rc; \
    bline_t *line = NULL; \
    bint_t col = 0; \
    bint_t char_count = 0; \
    if ((rc = (findfn)((mark), __VA_ARGS__, &line, &col, &char_count)) == MLBUF_OK) { \
        _mark_mark_move_inner((mark), line, col, 1); \
        if (optret_line) *optret_line = line; \
        if (optret_col) *optret_col = col; \
        if (optret_char_count) *optret_char_count = char_count; \
        return MLBUF_OK; \
    } \
    return rc; \
} while(0)

#define MLBUF_MARK_IMPLEMENT_MOVE_VIA_FIND(mark, findfn, ...) do { \
    int rc; \
    bline_t *line = NULL; \
    bint_t col = 0; \
    bint_t char_count = 0; \
    if ((rc = (findfn)((mark), __VA_ARGS__, &line, &col, &char_count)) == MLBUF_OK) { \
        _mark_mark_move_inner((mark), line, col, 1); \
    } \
    return rc; \
} while(0)

#define MLBUF_MARK_IMPLEMENT_NUDGE_VIA_FIND(mark, findfn, ...) do { \
    int rc; \
    bline_t *line = NULL; \
    bint_t col = 0; \
    bint_t char_count = 0; \
    mark_t *tmark = NULL; \
    mark_clone((mark), &tmark); \
    mark_move_by(tmark, 1); \
    if ((rc = (findfn)(tmark, __VA_ARGS__, &line, &col, &char_count)) == MLBUF_OK) { \
        _mark_mark_move_inner((mark), line, col, 1); \
    } \
    mark_destroy(tmark); \
    return rc; \
} while(0)

int mark_move_next_str(mark_t *self, char *str, bint_t str_len) {
    MLBUF_MARK_IMPLEMENT_MOVE_VIA_FIND(self, mark_find_next_str, str, str_len);
}

int mark_move_prev_str(mark_t *self, char *str, bint_t str_len) {
    MLBUF_MARK_IMPLEMENT_MOVE_VIA_FIND(self, mark_find_prev_str, str, str_len);
}

int mark_move_next_cre(mark_t *self, pcre2_code *cre) {
    MLBUF_MARK_IMPLEMENT_MOVE_VIA_FIND(self, mark_find_next_cre, cre);
}

int mark_move_prev_cre(mark_t *self, pcre2_code *cre) {
    MLBUF_MARK_IMPLEMENT_MOVE_VIA_FIND(self, mark_find_prev_cre, cre);
}

int mark_move_next_re(mark_t *self, char *re, bint_t re_len) {
    MLBUF_MARK_IMPLEMENT_MOVE_VIA_FIND(self, mark_find_next_re, re, re_len);
}

int mark_move_prev_re(mark_t *self, char *re, bint_t re_len) {
    MLBUF_MARK_IMPLEMENT_MOVE_VIA_FIND(self, mark_find_prev_re, re, re_len);
}

int mark_move_next_str_nudge(mark_t *self, char *str, bint_t str_len) {
    MLBUF_MARK_IMPLEMENT_NUDGE_VIA_FIND(self, mark_find_next_str, str, str_len);
}

int mark_move_next_cre_nudge(mark_t *self, pcre2_code *cre) {
    MLBUF_MARK_IMPLEMENT_NUDGE_VIA_FIND(self, mark_find_next_cre, cre);
}

int mark_move_next_re_nudge(mark_t *self, char *re, bint_t re_len) {
    MLBUF_MARK_IMPLEMENT_NUDGE_VIA_FIND(self, mark_find_next_re, re, re_len);
}

int mark_move_bracket_pair(mark_t *self, bint_t max_chars) {
    MLBUF_MARK_IMPLEMENT_MOVE_VIA_FIND(self, mark_find_bracket_pair, max_chars);
}

int mark_move_bracket_top(mark_t *self, bint_t max_chars) {
    MLBUF_MARK_IMPLEMENT_MOVE_VIA_FIND(self, mark_find_bracket_top, max_chars);
}

int mark_move_next_str_ex(mark_t *self, char *str, bint_t str_len, bline_t **optret_line, bint_t *optret_col, bint_t *optret_char_count) {
    MLBUF_MARK_IMPLEMENT_MOVE_VIA_FIND_EX(self, mark_find_next_str, str, str_len);
}

int mark_move_prev_str_ex(mark_t *self, char *str, bint_t str_len, bline_t **optret_line, bint_t *optret_col, bint_t *optret_char_count) {
    MLBUF_MARK_IMPLEMENT_MOVE_VIA_FIND_EX(self, mark_find_prev_str, str, str_len);
}

int mark_move_next_cre_ex(mark_t *self, pcre2_code *cre, bline_t **optret_line, bint_t *optret_col, bint_t *optret_char_count) {
    MLBUF_MARK_IMPLEMENT_MOVE_VIA_FIND_EX(self, mark_find_next_cre, cre);
}

int mark_move_prev_cre_ex(mark_t *self, pcre2_code *cre, bline_t **optret_line, bint_t *optret_col, bint_t *optret_char_count) {
    MLBUF_MARK_IMPLEMENT_MOVE_VIA_FIND_EX(self, mark_find_prev_cre, cre);
}

int mark_move_next_re_ex(mark_t *self, char *re, bint_t re_len, bline_t **optret_line, bint_t *optret_col, bint_t *optret_char_count) {
    MLBUF_MARK_IMPLEMENT_MOVE_VIA_FIND_EX(self, mark_find_next_re, re, re_len);
}

int mark_move_prev_re_ex(mark_t *self, char *re, bint_t re_len, bline_t **optret_line, bint_t *optret_col, bint_t *optret_char_count) {
    MLBUF_MARK_IMPLEMENT_MOVE_VIA_FIND_EX(self, mark_find_prev_re, re, re_len);
}

int mark_move_bracket_pair_ex(mark_t *self, bint_t max_chars, bline_t **optret_line, bint_t *optret_col, bint_t *optret_char_count) {
    MLBUF_MARK_IMPLEMENT_MOVE_VIA_FIND_EX(self, mark_find_bracket_pair, max_chars);
}

int mark_move_bracket_top_ex(mark_t *self, bint_t max_chars, bline_t **optret_line, bint_t *optret_col, bint_t *optret_char_count) {
    MLBUF_MARK_IMPLEMENT_MOVE_VIA_FIND_EX(self, mark_find_bracket_top, max_chars);
}

// Return 1 if mark is at a word boundary. If side <= -1, return 1 only for
// left word boundary (i.e., \W\w). If side >= 1, return 1 only for right word
// boundary (i.e., \w\W). If side == 0, return 1 for either case.
int mark_is_at_word_bound(mark_t *self, int side) {
    uint32_t before, after;
    MLBUF_BLINE_ENSURE_CHARS(self->bline);
    before = self->col > 0 && self->col - 1 < self->bline->char_count ? self->bline->chars[self->col - 1].ch : 0;
    after  = self->col < self->bline->char_count ? self->bline->chars[self->col].ch : 0;
    if (side <= -1 || side == 0) {
        // If before is bol or non-word, and after is word
        if ((before == 0 || !(isalnum(before) || before == '_'))
            && (isalnum(after) || after == '_')
        ) {
            return 1;
        }
    }
    if (side >= 1 || side == 0) {
        // If after is eol or non-word, and before is word
        if ((after == 0 || !(isalnum(after) || after == '_'))
            && (isalnum(before) || before == '_')
        ) {
            return 1;
        }
    }
    return 0;
}

// Set ovector for capturing substrs
int mark_set_pcre_capture(int *rc, PCRE2_SIZE *ovector, int ovector_size) {
    if (rc == NULL || ovector == NULL || ovector_size == 0) {
        rc = NULL;
        pcre_ovector = NULL;
        pcre_ovector_size = 0;
        return MLBUF_OK;
    } else if (rc != NULL && ovector != NULL && ovector_size >= 3 && ovector_size % 3 == 0) {
        pcre_rc = rc;
        pcre_ovector = ovector;
        pcre_ovector_size = ovector_size;
        return MLBUF_OK;
    }
    pcre_rc = NULL;
    pcre_ovector = NULL;
    pcre_ovector_size = 0;
    return MLBUF_ERR;
}

// Return char after mark, or 0 if at eol.
int mark_get_char_after(mark_t *self, uint32_t *ret_char) {
    if (mark_is_at_eol(self)) {
        *ret_char = 0;
    } else {
        MLBUF_BLINE_ENSURE_CHARS(self->bline);
        *ret_char = self->bline->chars[self->col].ch;
    }
    return MLBUF_OK;
}

// Return char before mark, or 0 if at bol.
int mark_get_char_before(mark_t *self, uint32_t *ret_char) {
    if (mark_is_at_bol(self)) {
        *ret_char = 0;
    } else {
        MLBUF_BLINE_ENSURE_CHARS(self->bline);
        *ret_char = self->bline->chars[self->col - 1].ch;
    }
    return MLBUF_OK;
}

// Return 1 if mark is after col, else 0. Lefty marks are considered 'after' col
// if `mark->col > col`. Righty marks (the default) are considered 'after' col
// if `mark->col >= col`.
int mark_is_after_col_minus_lefties(mark_t *self, bint_t col) {
    if (self->lefty) {
        return self->col > col ? 1 : 0;
    }
    return self->col >= col ? 1 : 0;
}

// Sort marks a and b as optret_first and optret_second
// Return -1 if a comes before b
//         1 if a comes after b
//         0 if a and b are at the same position
int mark_cmp(mark_t *a, mark_t *b, mark_t **optret_first, mark_t **optret_second) {
    int retval;
    mark_t *first, *second;
    if (mark_is_gt(b, a)) {
        first = a;
        second = b;
        retval = -1;
    } else if (mark_is_gt(a, b)) {
        first = b;
        second = a;
        retval = 1;
    } else {
        first = a;
        second = b;
        retval = 0;
    }
    if (optret_first)  *optret_first = first;
    if (optret_second) *optret_second = second;
    return retval;
}

int mark_block_insert_before(mark_t *self, char *data, bint_t data_len) {
    char *data_cur, *data_nl;
    bint_t data_rem_len, line_len, char_len, del_len, prefix_col;
    mark_t *m;

    m = self;

    prefix_col = m->col;
    data_cur = data;
    data_rem_len = data_len;
    while (1) {
        // Get line of data
        data_nl = memchr(data_cur, '\n', data_rem_len);
        line_len = data_nl ? (bint_t)(data_nl - data_cur) : data_rem_len;
        char_len = utf8_str_length(data_cur, line_len);

        // Replace up to char_len-worth of line
        del_len = MLBUF_MIN(m->bline->char_count - m->col, char_len);
        buffer_replace_w_bline(m->bline->buffer, m->bline, m->col, del_len, data_cur, line_len);

        // Advance
        if (!data_nl) break;
        data_cur = data_nl + 1;
        data_rem_len -= line_len + 1;

        // Ensure next line exists
        if (m->bline->next) {
            mark_move_to_w_bline(m, m->bline->next, 0);
        } else {
            mark_move_eol(m);
            mark_insert_before(m, "\n", 1);
        }

        // Ensure mark at prefix_col
        if (m->bline->char_count < prefix_col) {
            mark_move_eol(m);
            while (m->bline->char_count < prefix_col) mark_insert_before(m, " ", 1);
        }
        _mark_mark_move_inner(m, m->bline, prefix_col, 0);
    }

    return MLBUF_OK;
}

int mark_block_get_between(mark_t *self, mark_t *other, char **ret_str, bint_t *ret_str_len) {
    return mark_block_x_between(self, other, 0, ret_str, ret_str_len);
}

int mark_block_delete_between(mark_t *self, mark_t *other) {
    return mark_block_x_between(self, other, 1, NULL, NULL);
}

int mark_block_is_between(mark_t *self, mark_t *ma, mark_t *mb) {
    mark_t *a, *b;
    bint_t s, e;
    mark_cmp(ma, mb, &a, &b);
    s = MLBUF_MIN(a->col, b->col);
    e = MLBUF_MAX(a->col, b->col);
    return (
           self->bline->line_index >= a->bline->line_index
        && self->bline->line_index <= b->bline->line_index
        && self->col >= s
        && self->col <  e
    ) ? 1 : 0;
}

int mark_block_get_top_left(mark_t *self, mark_t *other, bline_t **ret_bline, bint_t *ret_col) {
    mark_t *a, *b;
    mark_cmp(self, other, &a, &b);
    *ret_bline = a->bline;
    *ret_col =  MLBUF_MIN(a->col, b->col);
    return MLBUF_OK;
}

static int mark_block_x_between(mark_t *self, mark_t *other, int del, char **optret_str, bint_t *optret_str_len) {
    str_t buf = {0};
    char *s;
    bint_t slen, start_col, end_col, end_col_adj, ig;
    bline_t *bline;
    mark_t *a, *b;

    if (mark_cmp(self, other, &a, &b) == 0 || a->col == b->col) {
        if (!del) {
            *optret_str = strdup("");
            *optret_str_len = 0;
        }
        return MLBUF_OK;
    }

    start_col = MLBUF_MIN(a->col, b->col);
    end_col = MLBUF_MAX(a->col, b->col);

    for (bline = a->bline; bline != b->bline->next; bline = bline->next) {
        if (start_col < bline->char_count) {
            end_col_adj = MLBUF_MIN(end_col, bline->char_count);
            if (del) {
                while (end_col_adj - start_col > (bint_t)buf.len) str_append_char(&buf, ' ');
                buffer_replace_w_bline(bline->buffer, bline, start_col, end_col_adj - start_col, buf.data, end_col_adj - start_col);
            } else {
                buffer_substr(bline->buffer, bline, start_col, bline, end_col_adj, &s, &slen, &ig);
                if (slen > 0) str_append_len(&buf, s, slen);
            }
        }
        if (!del && bline != b->bline->next) {
            str_append_char(&buf, '\n');
        }
    }

    if (del) {
        str_free(&buf);
    } else {
        *optret_str = buf.data;
        *optret_str_len = buf.len;
    }
    return MLBUF_OK;
}

// Find first occurrence of match according to matchfn. Search backwards if
// reverse is truthy.
static int mark_find_match(mark_t *self, mark_find_match_fn matchfn, void *u1, void *u2, int reverse, bline_t **ret_line, bint_t *ret_col, bint_t *ret_num_chars) {
    bline_t *search_line = NULL;
    char *match = NULL;
    bint_t look_offset = 0;
    bint_t match_col = 0;
    bint_t match_col_end = 0;
    bint_t max_offset = 0;
    bint_t match_len = 0;
    search_line = self->bline;
    *ret_line = NULL;
    if (reverse) {
        if (self->col <= 0) {
            // At bol, so look on prev line
            search_line = search_line->prev;
            if (!search_line) return MLBUF_ERR;
            look_offset = 0;
            max_offset = search_line->data_len;
        } else {
            look_offset = 0;
            MLBUF_BLINE_ENSURE_CHARS(search_line);
            max_offset = search_line->chars[self->col - 1].index;
        }
    } else {
        MLBUF_BLINE_ENSURE_CHARS(search_line);
        if (self->col >= search_line->char_count) {
            // At eol, so look on next line
            search_line = search_line->next;
            if (!search_line) return MLBUF_ERR;
            look_offset = 0;
            max_offset = search_line->data_len;
        } else {
            look_offset = self->col < search_line->char_count ? search_line->chars[self->col].index : search_line->data_len;
            max_offset = search_line->data_len;
        }
    }
    while (search_line) {
        match = matchfn(search_line->data, search_line->data_len, look_offset, max_offset, u1, u2, &match_len);
        if (match != NULL) {
            bline_get_col(search_line, (bint_t)(match - search_line->data), &match_col);
            bline_get_col(search_line, (bint_t)((match + match_len) - search_line->data), &match_col_end);
            *ret_line = search_line;
            *ret_col = match_col;
            *ret_num_chars = match_col_end - match_col;
            return MLBUF_OK;
        }
        search_line = reverse ? search_line->prev : search_line->next;
        if (search_line) {
            look_offset = 0;
            max_offset = search_line->data_len;
        }
    }
    return MLBUF_ERR;
}

// Move mark to target:col, setting target_col if do_set_target is truthy
void _mark_mark_move_inner(mark_t *mark, bline_t *bline_target, bint_t col, int do_set_target) {
    int is_changing_line;
    is_changing_line = mark->bline != bline_target ? 1 : 0;
    if (is_changing_line) {
        DL_DELETE(mark->bline->marks, mark);
        mark->bline = bline_target;
    }
    MLBUF_BLINE_ENSURE_CHARS(mark->bline);
    mark->col = MLBUF_MIN(mark->bline->char_count, MLBUF_MAX(0, col));
    if (do_set_target) {
        mark->target_col = mark->col;
    }
    if (is_changing_line) {
        DL_APPEND(bline_target->marks, mark);
    }
}

// Return the last occurrence of a match given a forward-searching matchfn
static char *mark_find_match_prev(char *haystack, bint_t haystack_len, bint_t look_offset, bint_t max_offset, mark_find_match_fn matchfn, void *u1, void *u2) {
    char *match;
    char *last_match;
    bint_t match_len;
    last_match = NULL;
    while (1) {
        match = matchfn(haystack, haystack_len, look_offset, max_offset, u1, u2, &match_len);
        if (match == NULL) {
            return last_match;
        }
        if (match - haystack > max_offset) {
            return last_match;
        }
        // Override match_len to 1. Reasoning: If we have a haystack like
        // 'banana' and our re is 'ana', using the actual match_len for the
        // next search offset would skip the 2nd 'ana' match.
        match_len = 1;
        look_offset = (bint_t)(match - haystack) + match_len;
        if (look_offset + match_len > haystack_len) {
            return match;
        }
        last_match = match;
    }
}

// Find uncompiled regex from mark. Search backwards if reverse is truthy.
static int mark_find_re(mark_t *self, char *re, bint_t re_len, int reverse, bline_t **ret_line, bint_t *ret_col, bint_t *ret_num_chars) {
    int rc;
    char *regex;
    pcre2_code *cre;
    int errcode;
    PCRE2_SIZE erroffset;
    MLBUF_MAKE_GT_EQ0(re_len);
    regex = malloc(re_len + 1);
    snprintf(regex, re_len + 1, "%s", re);
    cre = pcre2_compile((PCRE2_SPTR)regex, (PCRE2_SIZE)strlen(regex), PCRE2_CASELESS, &errcode, &erroffset, NULL);
    if (cre == NULL) {
        // TODO log error
        free(regex);
        return MLBUF_ERR;
    }
    if (reverse) {
        rc = mark_find_prev_cre(self, cre, ret_line, ret_col, ret_num_chars);
    } else {
        rc = mark_find_next_cre(self, cre, ret_line, ret_col, ret_num_chars);
    }
    pcre2_code_free(cre);
    free(regex);
    return rc;
}

static char *mark_find_next_str_matchfn(char *haystack, bint_t haystack_len, bint_t look_offset, bint_t max_offset, void *needle, void *needle_len, bint_t *ret_needle_len) {
    if (ret_needle_len) *ret_needle_len = *((bint_t*)needle_len);
    if (look_offset >= haystack_len) return NULL;
    return memmem(haystack + look_offset, haystack_len - look_offset, needle, *((bint_t*)needle_len));
}

static char *mark_find_prev_str_matchfn(char *haystack, bint_t haystack_len, bint_t look_offset, bint_t max_offset, void *needle, void *needle_len, bint_t *ret_needle_len) {
    return mark_find_match_prev(haystack, haystack_len, look_offset, max_offset, mark_find_next_str_matchfn, needle, needle_len);
}

static char *mark_find_next_cre_matchfn(char *haystack, bint_t haystack_len, bint_t look_offset, bint_t max_offset, void *cre, void *unused, bint_t *ret_needle_len) {
    int rc;
    PCRE2_SIZE substrs[3];
    int *use_rc;
    PCRE2_SIZE *use_substrs;
    int use_substrs_size;
    if (!haystack || haystack_len == 0) {
        haystack = "";
        haystack_len = 0;
    }
    if (pcre_ovector) {
        use_substrs = pcre_ovector;
        use_substrs_size = pcre_ovector_size;
        use_rc = pcre_rc;
    } else {
        use_substrs = substrs;
        use_substrs_size = 3;
        use_rc = &rc;
    }
    if ((*use_rc = pcre2_match((pcre2_code *)cre, (PCRE2_SPTR)haystack, (PCRE2_SIZE)haystack_len, (PCRE2_SIZE)look_offset, 0, pcre2_md, NULL)) >= 0) {
        memcpy(use_substrs, pcre2_get_ovector_pointer(pcre2_md), use_substrs_size * sizeof(PCRE2_SIZE));
        if (ret_needle_len) *ret_needle_len = (bint_t)(use_substrs[1] - use_substrs[0]);
        return haystack + use_substrs[0];
    }
    return NULL;
}

static char *mark_find_prev_cre_matchfn(char *haystack, bint_t haystack_len, bint_t look_offset, bint_t max_offset, void *cre, void *unused, bint_t *ret_needle_len) {
    return mark_find_match_prev(haystack, haystack_len, look_offset, max_offset, mark_find_next_cre_matchfn, cre, unused);
}
