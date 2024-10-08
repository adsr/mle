#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <utlist.h>
#include <inttypes.h>
#include <wchar.h>
#include "mlbuf.h"

static int _buffer_open_mmap(buffer_t *self, int fd, size_t size);
static int _buffer_open_read(buffer_t *self, int fd, size_t size);
static int _buffer_bline_unslab(bline_t *self);
static void _buffer_stat(buffer_t *self);
static int _buffer_baction_do(buffer_t *self, bline_t *bline, baction_t *action, int is_redo, bint_t *opt_repeat_offset);
static int _buffer_update(buffer_t *self, baction_t *action);
static int _buffer_undo(buffer_t *self, int by_group);
static int _buffer_redo(buffer_t *self, int by_group);
static int _buffer_truncate_undo_stack(buffer_t *self, baction_t *action_from);
static int _buffer_add_to_undo_stack(buffer_t *self, baction_t *action);
static int _buffer_apply_styles_all(bline_t *bline, bint_t min_nlines);
static int _buffer_match_srule(bline_t *bline, bint_t look_offset, srule_t *srule, int end_rule, bint_t *ret_start, bint_t *ret_stop);
static void _buffer_bline_reset_styles(bline_t *bline);
static void _buffer_bline_style(bline_t *bline, bint_t start, bint_t stop, sblock_t *style);
static bline_t *_buffer_bline_new(buffer_t *self);
static int _buffer_bline_free(bline_t *bline, bline_t *maybe_mark_line, bint_t col_delta);
static bline_t *_buffer_bline_break(bline_t *bline, bint_t col);
static void _buffer_find_end_pos(bline_t *start_line, bint_t start_col, bint_t num_chars, bline_t **ret_end_line, bint_t *ret_end_col, bint_t *ret_safe_num_chars);
static void _buffer_bline_replace(bline_t *bline, bint_t start_col, char *data, bint_t data_len, str_t *del_data);
static bint_t _buffer_bline_insert(bline_t *bline, bint_t col, char *data, bint_t data_len, int move_marks);
static bint_t _buffer_bline_delete(bline_t *bline, bint_t col, bint_t num_chars);
static bint_t _buffer_bline_col_to_index(bline_t *bline, bint_t col);
static bint_t _buffer_bline_index_to_col(bline_t *bline, bint_t index);
static int _buffer_munmap(buffer_t *self);
static int _baction_destroy(baction_t *action);
static void _bline_advance_col(bline_t **self, bint_t *col);

// Make a new buffer and return it
buffer_t *buffer_new(void) {
    buffer_t *buffer;
    bline_t *bline;
    buffer = calloc(1, sizeof(buffer_t));
    buffer->tab_width = 4;
    bline = _buffer_bline_new(buffer);
    buffer->first_line = bline;
    buffer->last_line = bline;
    buffer->line_count = 1;
    buffer->mmap_fd = -1;
    return buffer;
}

// Wrapper for buffer_new + buffer_open
buffer_t *buffer_new_open(char *path) {
    buffer_t *self;
    int rc;
    self = buffer_new();
    if ((rc = buffer_open(self, path)) != MLBUF_OK) {
        buffer_destroy(self);
        return NULL;
    }
    return self;
}

// Read buffer from path
int buffer_open(buffer_t *self, char *path) {
    int rc;
    struct stat st;
    int fd;
    fd = -1;

    rc = MLBUF_OK;
    do {
        // Exit early if path is empty
        if (!path || strlen(path) < 1) {
            rc = MLBUF_ERR;
            break;
        }

        // Open file for reading
        if ((fd = open(path, O_RDONLY)) < 0) {
            rc = MLBUF_ERR;
            break;
        }

        // Stat file
        if (fstat(fd, &st) < 0) {
            rc = MLBUF_ERR;
            break;
        }

        // Read or mmap file into buffer
        self->is_in_open = 1;
        if (st.st_size >= MLBUF_LARGE_FILE_SIZE) {
            if (_buffer_open_mmap(self, fd, st.st_size) != MLBUF_OK) {
                // mmap failed so fallback to normal read
                if (_buffer_open_read(self, fd, st.st_size) != MLBUF_OK) {
                    rc = MLBUF_ERR;
                    break;
                }
            }
        } else {
            if (_buffer_open_read(self, fd, st.st_size) != MLBUF_OK) {
                rc = MLBUF_ERR;
                break;
            }
        }
        self->is_in_open = 0;
    } while(0);

    if (fd >= 0) close(fd);

    if (rc == MLBUF_ERR) return rc;

    // Set path
    if (self->path) free(self->path);
    self->path = strdup(path);
    self->is_unsaved = 0;

    // Remember stat
    _buffer_stat(self);

    return MLBUF_OK;
}

// Write buffer to path
int buffer_save(buffer_t *self) {
    return buffer_save_as(self, self->path, NULL);
}

// Write buffer to specified path
int buffer_save_as(buffer_t *self, char *path, bint_t *optret_nbytes) {
    FILE *fp;
    size_t nbytes;

    if (optret_nbytes) *optret_nbytes = 0;

    // Exit early if path is empty
    if (!path || strlen(path) < 1) {
        return MLBUF_ERR;
    }

    // Open file for writing
    if (!(fp = fopen(path, "wb"))) {
        return MLBUF_ERR;
    }

    // Write data to file
    nbytes = 0;
    buffer_write_to_file(self, fp, &nbytes);
    fclose(fp);
    if (optret_nbytes) *optret_nbytes = (bint_t)nbytes;
    if ((bint_t)nbytes != self->byte_count) return MLBUF_ERR;

    // Set path
    if (self->path != path) {
        if (self->path) free(self->path);
        self->path = strdup(path);
    }
    self->is_unsaved = 0;

    // Remember stat
    _buffer_stat(self);

    return MLBUF_OK;
}

// Write buffer data to FILE*
int buffer_write_to_file(buffer_t *self, FILE *fp, size_t *optret_nbytes) {
    return buffer_write_to_fd(self, fileno(fp), optret_nbytes);
}

// Write buffer data to file descriptor
int buffer_write_to_fd(buffer_t *self, int fd, size_t *optret_nbytes) {
    bline_t *bline;
    size_t nbytes;
    ssize_t write_rc;
    nbytes = 0;
    #define MLBUF_BUFFER_WRITE_CHECK(pd, pl) do { \
        write_rc = write(fd, (pd), (pl)); \
        if (write_rc < (pl)) { \
            return MLBUF_ERR; \
        } \
        nbytes += write_rc; \
    } while(0)
    for (bline = self->first_line; bline; bline = bline->next) {
        if (bline->data_len > 0) MLBUF_BUFFER_WRITE_CHECK(bline->data, bline->data_len);
        if (bline->next)         MLBUF_BUFFER_WRITE_CHECK("\n", 1);
    }
    if (optret_nbytes) *optret_nbytes = nbytes;
    return MLBUF_OK;
}

// Free a buffer
int buffer_destroy(buffer_t *self) {
    bline_t *line;
    bline_t *line_tmp;
    baction_t *action;
    baction_t *action_tmp;
    char c;
    for (line = self->last_line; line; ) {
        line_tmp = line->prev;
        _buffer_bline_free(line, NULL, 0);
        line = line_tmp;
    }
    if (self->data) free(self->data);
    if (self->path) free(self->path);
    DL_FOREACH_SAFE(self->actions, action, action_tmp) {
        DL_DELETE(self->actions, action);
        _baction_destroy(action);
    }
    for (c = 'a'; c <= 'z'; c++) buffer_register_clear(self, c);
    _buffer_munmap(self);
    if (self->slabbed_blines) free(self->slabbed_blines);
    if (self->slabbed_chars) free(self->slabbed_chars);
    free(self);
    return MLBUF_OK;
}

// Add a mark to this buffer and return it
mark_t *buffer_add_mark(buffer_t *self, bline_t *maybe_line, bint_t maybe_col) {
    return buffer_add_mark_ex(self, '\0', maybe_line, maybe_col);
}

// If letter is [a-z], add a lettered mark and return it.
// If letter is \0, add a non-lettered mark and return it.
// Otherwise do nothing and return NULL.
mark_t *buffer_add_mark_ex(buffer_t *self, char letter, bline_t *maybe_line, bint_t maybe_col) {
    mark_t *mark;
    mark_t *mark_tmp;
    if (!((letter >= 'a' && letter <= 'z') || letter == '\0')) {
        return NULL;
    }
    mark = calloc(1, sizeof(mark_t));
    mark->letter = letter;
    MLBUF_MAKE_GT_EQ0(maybe_col);
    if (maybe_line != NULL) {
        MLBUF_BLINE_ENSURE_CHARS(maybe_line);
        mark->bline = maybe_line;
        mark->col = maybe_col < 0 ? 0 : MLBUF_MIN(maybe_line->char_count, maybe_col);
    } else {
        mark->bline = self->first_line;
        mark->col = 0;
    }
    DL_APPEND(mark->bline->marks, mark);
    if (mark->letter) {
        if ((mark_tmp = MLBUF_LETT_MARK(self, mark->letter)) != NULL) {
            buffer_destroy_mark(self, mark_tmp);
        }
        MLBUF_LETT_MARK(self, mark->letter) = mark;
    }
    return mark;
}

// Return lettered mark or NULL if it does not exist
int buffer_get_lettered_mark(buffer_t *self, char letter, mark_t **ret_mark) {
    MLBUF_ENSURE_AZ(letter);
    *ret_mark = MLBUF_LETT_MARK(self, letter);
    return MLBUF_OK;
}

// Remove mark from buffer and free it, removing any range srules that use it
int buffer_destroy_mark(buffer_t *self, mark_t *mark) {
    srule_node_t *node;
    srule_node_t *node_tmp;
    DL_DELETE(mark->bline->marks, mark);
    if (mark->letter) MLBUF_LETT_MARK(self, mark->letter) = NULL;
    DL_FOREACH_SAFE(self->range_srules, node, node_tmp) {
        if (node->srule->range_a == mark
            || node->srule->range_b == mark
        ) {
            buffer_remove_srule(self, node->srule);
        }
    }
    free(mark);
    return MLBUF_OK;
}

// Get buffer contents and length
int buffer_get(buffer_t *self, char **ret_data, bint_t *ret_data_len) {
    bline_t *bline;
    char *data_cursor;
    bint_t alloc_size;
    if (self->is_data_dirty) {
        // Refresh self->data
        alloc_size = self->byte_count + 2;
        self->data = self->data != NULL
            ? realloc(self->data, alloc_size)
            : malloc(alloc_size);
        data_cursor = self->data;
        for (bline = self->first_line; bline != NULL; bline = bline->next) {
            if (bline->data_len > 0) {
                memcpy(data_cursor, bline->data, bline->data_len);
                data_cursor += bline->data_len;
                self->data_len += bline->data_len;
            }
            if (bline->next) {
                *data_cursor = '\n';
                data_cursor += 1;
            }
        }
        *data_cursor = '\0';
        self->data_len = (bint_t)(data_cursor - self->data);
        self->is_data_dirty = 0;
    }
    *ret_data = self->data;
    *ret_data_len = self->data_len;
    return MLBUF_OK;
}

int buffer_clear(buffer_t *self) {
    return buffer_delete(self, 0, self->byte_count);
}

// Set buffer contents
int buffer_set(buffer_t *self, char *data, bint_t data_len) {
    int rc;
    MLBUF_MAKE_GT_EQ0(data_len);
    if ((rc = buffer_clear(self)) != MLBUF_OK) {
        return rc;
    }
    rc = buffer_insert(self, 0, data, data_len, NULL);
    if (self->actions) _buffer_truncate_undo_stack(self, self->actions);
    return rc;
}

// Set buffer contents more efficiently
int buffer_set_mmapped(buffer_t *self, char *data, bint_t data_len) {
    bint_t nlines;
    bint_t line_num;
    bline_t *blines;
    bint_t data_remaining_len;
    char *data_cursor;
    char *data_newline;
    bint_t line_len;

    if (buffer_clear(self) != MLBUF_OK) {
        return MLBUF_ERR;
    }

    // Count number of lines
    nlines = 1;
    data_cursor = data;
    data_remaining_len = data_len;
    while (data_remaining_len > 0 && (data_newline = memchr(data_cursor, '\n', data_remaining_len)) != NULL) {
        data_remaining_len -= (bint_t)(data_newline - data_cursor) + 1;
        data_cursor = data_newline + 1;
        nlines += 1;
    }

    // Allocate blines and chars. These are freed in buffer_destroy.
    self->slabbed_chars = calloc(data_len, sizeof(bline_char_t));
    self->slabbed_blines = malloc(nlines * sizeof(bline_t));
    blines = self->slabbed_blines;

    // Populate blines
    line_num = 0;
    data_cursor = data;
    data_remaining_len = data_len;
    while (1) {
        data_newline = data_remaining_len > 0
            ? memchr(data_cursor, '\n', data_remaining_len)
            : NULL;
        line_len = data_newline ?
            (bint_t)(data_newline - data_cursor)
            : data_remaining_len;
        blines[line_num] = (bline_t){
            .buffer = self,
            .data = data_cursor,
            .data_len = line_len,
            .data_cap = line_len,
            .line_index = line_num,
            .char_count = line_len,
            .char_vwidth = line_len,
            .chars = (self->slabbed_chars + (data_len - data_remaining_len)),
            .chars_cap = line_len,
            .marks = NULL,
            .eol_rule = NULL,
            .is_chars_dirty = 1,
            .is_slabbed = 1,
            .is_data_slabbed = 1,
            .next = NULL,
            .prev = NULL
        };
        if (line_num > 0) {
            blines[line_num-1].next = blines + line_num;
            blines[line_num].prev = blines + (line_num-1);
        }
        if (data_newline) {
            data_remaining_len -= line_len + 1;
            data_cursor = data_newline + 1;
            line_num += 1;
        } else {
            break;
        }
    }

    if (self->first_line) _buffer_bline_free(self->first_line, NULL, 0);
    self->first_line = blines;
    self->last_line = blines + line_num;
    self->byte_count = data_len;
    self->line_count = line_num + 1;
    self->is_data_dirty = 1;
    return MLBUF_OK;
}

// Insert data into buffer given a buffer offset
int buffer_insert(buffer_t *self, bint_t offset, char *data, bint_t data_len, bint_t *optret_num_chars) {
    int rc;
    bline_t *start_line;
    bint_t start_col;
    MLBUF_MAKE_GT_EQ0(offset);

    // Find start line and col
    if ((rc = buffer_get_bline_col(self, offset, &start_line, &start_col)) != MLBUF_OK) {
        return rc;
    }

    return buffer_insert_w_bline(self, start_line, start_col, data, data_len, optret_num_chars);
}

// Insert data into buffer given a bline/col
int buffer_insert_w_bline(buffer_t *self, bline_t *start_line, bint_t start_col, char *data, bint_t data_len, bint_t *optret_num_chars) {
    bline_t *cur_line;
    bint_t cur_col;
    bline_t *new_line;
    char *data_cursor;
    char *data_newline;
    bint_t data_remaining_len;
    bint_t insert_len;
    bint_t num_lines_added;
    char *ins_data;
    bint_t ins_data_len;
    bint_t ins_data_nchars;
    baction_t *action;
    MLBUF_MAKE_GT_EQ0(data_len);

    // Exit early if no data
    if (data_len < 1) {
        return MLBUF_OK;
    }

    // Insert lines
    data_cursor = data;
    data_remaining_len = data_len;
    cur_line = start_line;
    cur_col = start_col;
    num_lines_added = 0;
    while (data_remaining_len > 0 && (data_newline = memchr(data_cursor, '\n', data_remaining_len)) != NULL) {
        insert_len = (bint_t)(data_newline - data_cursor);
        new_line = _buffer_bline_break(cur_line, cur_col);
        num_lines_added += 1;
        if (insert_len > 0) {
            _buffer_bline_insert(cur_line, cur_col, data_cursor, insert_len, 1);
        }
        data_remaining_len -= (data_newline - data_cursor) + 1;
        data_cursor = data_newline + 1;
        cur_line = new_line;
        cur_col = 0;
    }
    if (data_remaining_len > 0) {
        cur_col += _buffer_bline_insert(cur_line, cur_col, data_cursor, data_remaining_len, 1);
    }

    // Get inserted data
    buffer_substr(self, start_line, start_col, cur_line, cur_col, &ins_data, &ins_data_len, &ins_data_nchars);

    // Add baction
    action = calloc(1, sizeof(baction_t));
    action->type = MLBUF_BACTION_TYPE_INSERT;
    action->buffer = self;
    action->start_line = start_line;
    action->start_line_index = start_line->line_index;
    action->start_col = start_col;
    action->maybe_end_line = cur_line;
    action->maybe_end_line_index = action->start_line_index + num_lines_added;
    action->maybe_end_col = cur_col;
    action->byte_delta = (bint_t)ins_data_len;
    action->char_delta = (bint_t)ins_data_nchars;
    action->line_delta = (bint_t)num_lines_added;
    action->data = ins_data;
    action->data_len = ins_data_len;
    _buffer_update(self, action);
    if (optret_num_chars) *optret_num_chars = ins_data_nchars;

    return MLBUF_OK;
}

// Delete data from buffer given an offset
int buffer_delete(buffer_t *self, bint_t offset, bint_t num_chars) {
    bline_t *start_line;
    bint_t start_col;
    MLBUF_MAKE_GT_EQ0(offset);
    buffer_get_bline_col(self, offset, &start_line, &start_col);
    return buffer_delete_w_bline(self, start_line, start_col, num_chars);
}

// Delete data from buffer given a bline/col
int buffer_delete_w_bline(buffer_t *self, bline_t *start_line, bint_t start_col, bint_t num_chars) {
    bline_t *end_line;
    bint_t end_col;
    bline_t *tmp_line;
    bline_t *swap_line;
    bline_t *next_line;
    bint_t tmp_len;
    char *del_data;
    bint_t del_data_len;
    bint_t del_data_nchars;
    bint_t num_lines_removed;
    bint_t safe_num_chars;
    bint_t orig_char_count;
    baction_t *action;
    MLBUF_MAKE_GT_EQ0(num_chars);

    // Find end line and col
    _buffer_find_end_pos(start_line, start_col, num_chars, &end_line, &end_col, &num_chars);

    // Exit early if there is nothing to delete
    MLBUF_BLINE_ENSURE_CHARS(self->last_line);
    if (start_line == end_line && start_col >= end_col) {
        return MLBUF_OK;
    } else if (start_line == self->last_line && start_col == self->last_line->char_count) {
        return MLBUF_OK;
    }

    // Get deleted data
    buffer_substr(self, start_line, start_col, end_line, end_col, &del_data, &del_data_len, &del_data_nchars);

    // Delete suffix starting at start_line:start_col
    MLBUF_BLINE_ENSURE_CHARS(start_line);
    safe_num_chars = MLBUF_MIN(num_chars, start_line->char_count - start_col);
    if (safe_num_chars > 0) {
        _buffer_bline_delete(start_line, start_col, safe_num_chars);
    }

    // Copy remaining portion of end_line to start_line:start_col
    MLBUF_BLINE_ENSURE_CHARS(start_line);
    orig_char_count = start_line->char_count;
    if (start_line != end_line && (tmp_len = end_line->data_len - _buffer_bline_col_to_index(end_line, end_col)) > 0) {
        _buffer_bline_insert(
            start_line,
            start_col,
            end_line->data + (end_line->data_len - tmp_len),
            tmp_len,
            0
        );
    }

    // Remove lines after start_line thru end_line
    // Relocate marks to end of start_line
    num_lines_removed = 0;
    swap_line = end_line->next;
    next_line = NULL;
    tmp_line = start_line->next;
    while (tmp_line != NULL && tmp_line != swap_line) {
        next_line = tmp_line->next;
        _buffer_bline_free(tmp_line, start_line, orig_char_count - end_col);
        num_lines_removed += 1;
        tmp_line = next_line;
    }
    start_line->next = swap_line;
    if (swap_line) swap_line->prev = start_line;

    // Add baction
    action = calloc(1, sizeof(baction_t));
    action->type = MLBUF_BACTION_TYPE_DELETE;
    action->buffer = self;
    action->start_line = start_line;
    action->start_line_index = start_line->line_index;
    action->start_col = start_col;
    action->byte_delta = -1 * (bint_t)del_data_len;
    action->char_delta = -1 * (bint_t)del_data_nchars;
    action->line_delta = -1 * (bint_t)num_lines_removed;
    action->data = del_data;
    action->data_len = del_data_len;
    _buffer_update(self, action);

    return MLBUF_OK;
}

// Replace num_chars in buffer at offset with data
int buffer_replace(buffer_t *self, bint_t offset, bint_t num_chars, char *data, bint_t data_len) {
    int rc;
    bline_t *start_line;
    bint_t start_col;
    MLBUF_MAKE_GT_EQ0(offset);

    // Find start line and col
    if ((rc = buffer_get_bline_col(self, offset, &start_line, &start_col)) != MLBUF_OK) {
        return rc;
    }

    return buffer_replace_w_bline(self, start_line, start_col, num_chars, data, data_len);
}

// Replace num_chars from start_line:start_col with data
int buffer_replace_w_bline(buffer_t *self, bline_t *start_line, bint_t start_col, bint_t del_chars, char *data, bint_t data_len) {
    bline_t *cur_line;
    bint_t cur_col;
    bint_t insert_rem;
    bint_t delete_rem;
    bint_t data_linelen;
    bint_t nchars_ins;
    bint_t nlines;
    char *data_cursor;
    char *data_newline;
    baction_t *action;
    str_t del_data = {0};

    // Replace data on common lines
    insert_rem = data_len;
    delete_rem = del_chars;
    cur_line = start_line;
    cur_col = start_col;
    data_cursor = data;
    nchars_ins = 0;
    nlines = 0;
    MLBUF_BLINE_ENSURE_CHARS(cur_line);
    while (insert_rem > 0 && delete_rem > (cur_line->char_count - cur_col)) {
        data_newline = memchr(data_cursor, '\n', insert_rem);
        data_linelen = data_newline ? (bint_t)(data_newline - data_cursor) : insert_rem;
        delete_rem -= cur_line->char_count - cur_col;
        _buffer_bline_replace(cur_line, cur_col, data_cursor, data_linelen, &del_data);
        nchars_ins += cur_line->char_count - cur_col;
        insert_rem -= data_linelen;
        data_cursor += data_linelen;
        cur_col = cur_line->char_count;
        if (data_newline && insert_rem >= 2 && delete_rem >= 2 && cur_line->next) {
            data_cursor += 1;
            insert_rem -= 1;
            delete_rem -= 1;
            cur_line = cur_line->next;
            cur_col = 0;
            nchars_ins += 1;
            str_append_len(&del_data, "\n", 1);
        } else {
            break;
        }
        nlines += 1;
        MLBUF_BLINE_ENSURE_CHARS(cur_line);
    }

    // Add delete baction
    if (del_data.len > 0) {
        action = calloc(1, sizeof(baction_t));
        action->type = MLBUF_BACTION_TYPE_DELETE;
        action->buffer = self;
        action->start_line = start_line;
        action->start_line_index = start_line->line_index;
        action->start_col = start_col;
        action->byte_delta = -1 * (bint_t)del_data.len;
        action->char_delta = -1 * (del_chars - delete_rem);
        action->line_delta = -1 * nlines;
        action->data = del_data.data;
        action->data_len = (bint_t)del_data.len;
        _buffer_update(self, action);
    }

    // Add insert baction
    if (data_len - insert_rem > 0) {
        action = calloc(1, sizeof(baction_t));
        action->type = MLBUF_BACTION_TYPE_INSERT;
        action->buffer = self;
        action->start_line = start_line;
        action->start_line_index = start_line->line_index;
        action->start_col = start_col;
        action->maybe_end_line = cur_line;
        action->maybe_end_line_index = action->start_line_index + nlines;
        action->maybe_end_col = cur_col;
        action->byte_delta = data_len - insert_rem;
        action->char_delta = nchars_ins;
        action->line_delta = nlines;
        action->data = strndup(data, action->byte_delta);
        action->data_len = action->byte_delta;
        _buffer_update(self, action);
    }

    // Delete left over data
    if (delete_rem > 0) {
        buffer_delete_w_bline(self, cur_line, cur_col, delete_rem);
    }

    // Insert left over data
    if (insert_rem > 0) {
        buffer_insert_w_bline(self, cur_line, cur_col, data_cursor, insert_rem, NULL);
    }

    return MLBUF_OK;
}

// Return a line given a line_index
int buffer_get_bline(buffer_t *self, bint_t line_index, bline_t **ret_bline) {
    return buffer_get_bline_w_hint(self, line_index, self->first_line, ret_bline);
}

// Return a line given a line_index, starting at hint and iterating outward
int buffer_get_bline_w_hint(buffer_t *self, bint_t line_index, bline_t *opt_hint, bline_t **ret_bline) {
    bline_t *fwd, *rev, *found;
    MLBUF_MAKE_GT_EQ0(line_index);

    if (!opt_hint) {
        opt_hint = self->first_line;
    }

    fwd = opt_hint;
    rev = opt_hint->prev;
    found = NULL;

    while (fwd || rev) {
        if (fwd) {
            if (fwd->line_index == line_index) {
                found = fwd;
                break;
            }
            fwd = fwd != fwd->next ? fwd->next : NULL;
        }
        if (rev) {
            if (rev->line_index == line_index) {
                found = rev;
                break;
            }
            rev = rev != rev->prev ? rev->prev : NULL;
        }
    }

    if (found) {
        *ret_bline = found;
        return MLBUF_OK;
    }

    *ret_bline = self->last_line;
    return MLBUF_ERR;
}

// Return a line and col for the given offset
int buffer_get_bline_col(buffer_t *self, bint_t offset, bline_t **ret_bline, bint_t *ret_col) {
    bline_t *tmp_line;
    bline_t *good_line = NULL;
    bint_t remaining_chars;
    MLBUF_MAKE_GT_EQ0(offset);

    // TODO convert to binary search for perf win
    remaining_chars = offset;
    for (tmp_line = self->first_line; tmp_line != NULL; tmp_line = tmp_line->next) {
        MLBUF_BLINE_ENSURE_CHARS(tmp_line);
        if (tmp_line->char_count >= remaining_chars) {
            *ret_bline = tmp_line;
            *ret_col = remaining_chars;
            return MLBUF_OK;
        } else {
            remaining_chars -= (tmp_line->char_count + 1); // Plus 1 for newline
        }
        good_line = tmp_line;
    }

    if (!good_line) good_line = self->first_line;
    *ret_bline = good_line;
    *ret_col = good_line->char_count;
    return MLBUF_OK;
}

// Return an offset given a line and col
int buffer_get_offset(buffer_t *self, bline_t *bline, bint_t col, bint_t *ret_offset) {
    bline_t *tmp_line;
    bint_t offset;
    MLBUF_MAKE_GT_EQ0(col);

    // TODO cache for perf win
    offset = 0;
    for (tmp_line = self->first_line; tmp_line != bline->next; tmp_line = tmp_line->next) {
        MLBUF_BLINE_ENSURE_CHARS(tmp_line);
        if (tmp_line == bline) {
            offset += MLBUF_MIN(tmp_line->char_count, col);
            break;
        } else {
            offset += tmp_line->char_count + 1; // Plus 1 for newline
        }
    }

    *ret_offset = offset;
    return MLBUF_OK;
}

// Add a style rule to the buffer
int buffer_add_srule(buffer_t *self, srule_t *srule) {
    srule_node_t *node;
    node = calloc(1, sizeof(srule_node_t));
    node->srule = srule;
    if (srule->type == MLBUF_SRULE_TYPE_SINGLE || srule->type == MLBUF_SRULE_TYPE_MULTI) {
        DL_APPEND(self->srules, node);
    } else if (srule->type == MLBUF_SRULE_TYPE_RANGE) {
        srule->range_a->range_srule = srule;
        srule->range_b->range_srule = srule;
        DL_APPEND(self->range_srules, node);
        return MLBUF_OK; // range styles get applied in bview_draw
    } else {
        free(node);
        return MLBUF_ERR;
    }
    return buffer_apply_styles(self, self->first_line, self->line_count - 1);
}

// Remove a style rule from the buffer
int buffer_remove_srule(buffer_t *self, srule_t *srule) {
    int found;
    srule_node_t **head;
    srule_node_t *node;
    srule_node_t *node_tmp;
    if (srule->type == MLBUF_SRULE_TYPE_SINGLE || srule->type == MLBUF_SRULE_TYPE_MULTI) {
        head = &self->srules;
    } else if (srule->type == MLBUF_SRULE_TYPE_RANGE) {
        head = &self->range_srules;
    } else {
        return MLBUF_ERR;
    }
    found = 0;
    DL_FOREACH_SAFE(*head, node, node_tmp) {
        if (node->srule != srule) continue;
        if (srule->type == MLBUF_SRULE_TYPE_RANGE) {
            srule->range_a->range_srule = NULL;
            srule->range_b->range_srule = NULL;
        }
        DL_DELETE(*head, node);
        free(node);
        found = 1;
        break;
    }
    if (!found) return MLBUF_ERR;
    if (srule->type == MLBUF_SRULE_TYPE_RANGE) {
        return MLBUF_OK; // range styles get applied in bview_draw
    }
    return buffer_apply_styles(self, self->first_line, self->line_count - 1);
}

// Set callback to cb. Pass in NULL to unset callback.
int buffer_set_callback(buffer_t *self, buffer_callback_t cb, void *udata) {
    if (cb) {
        self->callback = cb;
        self->callback_udata = udata;
    } else {
        self->callback = NULL;
        self->callback_udata = NULL;
    }
    return MLBUF_OK;
}

// Set tab_width and recalculate all line char vwidths
int buffer_set_tab_width(buffer_t *self, int tab_width) {
    bline_t *tmp_line;
    if (tab_width < 1) {
        return MLBUF_ERR;
    }
    // TODO invalidate by incrementing a version scalar or something
    self->tab_width = tab_width;
    for (tmp_line = self->first_line; tmp_line; tmp_line = tmp_line->next) {
        bline_count_chars(tmp_line);
    }
    return MLBUF_OK;
}

// Return data from start_line:start_col thru end_line:end_col
int buffer_substr(buffer_t *self, bline_t *start_line, bint_t start_col, bline_t *end_line, bint_t end_col, char **ret_data, bint_t *ret_data_len, bint_t *ret_data_nchars) {
    char *data;
    bint_t data_len;
    bint_t data_size;
    bline_t *tmp_line;
    bint_t copy_len;
    bint_t copy_index;
    bint_t add_len;
    bint_t nchars;
    MLBUF_MAKE_GT_EQ0(start_col);
    MLBUF_MAKE_GT_EQ0(end_col);

    data = calloc(2, sizeof(char));
    data_len = 0;
    data_size = 2;
    nchars = 0;

    for (tmp_line = start_line; tmp_line != end_line->next; tmp_line = tmp_line->next) {
        // Get copy_index + copy_len
        // Also increment nchars
        if (start_line == end_line) {
            copy_index = _buffer_bline_col_to_index(start_line, start_col);
            copy_len = _buffer_bline_col_to_index(start_line, end_col) - copy_index;
            nchars += end_col - start_col;
        } else if (tmp_line == start_line) {
            copy_index = _buffer_bline_col_to_index(start_line, start_col);
            copy_len = tmp_line->data_len - copy_index;
            MLBUF_BLINE_ENSURE_CHARS(start_line);
            nchars += start_line->char_count - start_col;
        } else if (tmp_line == end_line) {
            copy_index = 0;
            copy_len = _buffer_bline_col_to_index(end_line, end_col);
            nchars += end_col;
        } else {
            copy_index = 0;
            copy_len = tmp_line->data_len;
            MLBUF_BLINE_ENSURE_CHARS(tmp_line);
            nchars += tmp_line->char_count;
        }

        // Add 1 for newline if not on end_line
        add_len = copy_len + (tmp_line != end_line ? 1 : 0); // Plus 1 for newline
        nchars += (tmp_line != end_line ? 1 : 0);

        // Copy add_len bytes from copy_index into data
        if (add_len > 0) {
            if (data_len + add_len + 1 > data_size) {
                data_size = data_len + add_len + 1; // Plus 1 for nullchar
                data = realloc(data, data_size);
            }
            if (copy_len > 0) {
                memcpy(data + data_len, tmp_line->data + copy_index, copy_len);
                data_len += copy_len;
            }
            if (tmp_line != end_line) {
                *(data + data_len) = '\n';
                data_len += 1;
            }
        }
    }

    *(data + data_len) = '\0';
    *ret_data = data;
    *ret_data_len = data_len;
    *ret_data_nchars = nchars;

    return MLBUF_OK;
}

// Undo an action
int buffer_undo(buffer_t *self) {
    return _buffer_undo(self, 0);
}

// Undo actions in last action_group
int buffer_undo_action_group(buffer_t *self) {
    return _buffer_undo(self, 1);
}

// Redo an action
int buffer_redo(buffer_t *self) {
    return _buffer_redo(self, 0);
}

// Redo actions in last action_group
int buffer_redo_action_group(buffer_t *self) {
    return _buffer_redo(self, 1);
}


// Toggle is_style_disabled
int buffer_set_styles_enabled(buffer_t *self, int is_enabled) {
    if (!self->is_style_disabled && !is_enabled) {
        self->is_style_disabled = 1;
    } else if (self->is_style_disabled && is_enabled) {
        self->is_style_disabled = 0;
        buffer_apply_styles(self, self->first_line, self->line_count);
    }
    return MLBUF_OK;
}

// Set buffer action_group pointer
int buffer_set_action_group_ptr(buffer_t *self, int *action_group) {
    self->action_group = action_group;
    return MLBUF_OK;
}

// Apply styles from start_line
int buffer_apply_styles(buffer_t *self, bline_t *start_line, bint_t line_delta) {
    bint_t min_nlines;
    srule_node_t *srule_node;
    int count_tmp;
    int srule_count;

    if (self->is_style_disabled) {
        return MLBUF_OK;
    }

    // min_nlines, minimum number of lines to style
    //     line_delta  < 0: 2 (start_line + 1)
    //     line_delta == 0: 1 (start_line)
    //     line_delta  > 0: 1 + line_delta (start_line + added lines)
    min_nlines = 1 + (line_delta < 0 ? 1 : line_delta);

    // Count current srules
    srule_count = 0;
    DL_COUNT(self->srules, srule_node, count_tmp);
    srule_count += count_tmp;

    // Apply rules if there are any, or if the number of rules changed
    if (srule_count > 0 || self->num_applied_srules != srule_count) {
        _buffer_apply_styles_all(start_line, min_nlines);
        self->num_applied_srules = srule_count;
    }

    return MLBUF_OK;
}

// Set register
int buffer_register_set(buffer_t *self, char reg, char *data, size_t data_len) {
    MLBUF_ENSURE_AZ(reg);
    str_set_len(MLBUF_REG_PTR(self, reg), data, data_len);
    return MLBUF_OK;
}

// Append to register
int buffer_register_append(buffer_t *self, char reg, char *data, size_t data_len) {
    MLBUF_ENSURE_AZ(reg);
    str_append_len(MLBUF_REG_PTR(self, reg), data, data_len);
    return MLBUF_OK;
}

// Prepend to register
int buffer_register_prepend(buffer_t *self, char reg, char *data, size_t data_len) {
    MLBUF_ENSURE_AZ(reg);
    str_prepend_len(MLBUF_REG_PTR(self, reg), data, data_len);
    return MLBUF_OK;
}

// Clear register
int buffer_register_clear(buffer_t *self, char reg) {
    MLBUF_ENSURE_AZ(reg);
    str_free(MLBUF_REG_PTR(self, reg));
    return MLBUF_OK;
}

// Get register, optionally allocating duplicate
int buffer_register_get(buffer_t *self, char reg, int dup, char **ret_data, size_t *ret_data_len) {
    str_t *sreg;
    MLBUF_ENSURE_AZ(reg);
    sreg = MLBUF_REG_PTR(self, reg);
    if (dup) {
        *ret_data = strndup(sreg->data, sreg->len);
        *ret_data_len = strlen(*ret_data);
    } else {
        *ret_data = sreg->len > 0 ? sreg->data : "";
        *ret_data_len = sreg->len;
    }
    return MLBUF_OK;
}

// Count multi-byte characters and character widths of this line
int bline_count_chars(bline_t *bline) {
    char *c;
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
            // Let null occupy 1 column
            if (char_w < 0 || ch == '\0') char_w = 1;
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
int bline_insert(bline_t *self, bint_t col, char *data, bint_t data_len, bint_t *ret_num_chars) {
    _bline_advance_col(&self, &col);
    return buffer_insert_w_bline(self->buffer, self, col, data, data_len, ret_num_chars);
}

// Delete data from a line
int bline_delete(bline_t *self, bint_t col, bint_t num_chars) {
    _bline_advance_col(&self, &col);
    return buffer_delete_w_bline(self->buffer, self, col, num_chars);
}

// Replace data on a line
int bline_replace(bline_t *self, bint_t col, bint_t num_chars, char *data, bint_t data_len) {
    _bline_advance_col(&self, &col);
    return buffer_replace_w_bline(self->buffer, self, col, num_chars, data, data_len);
}

// Return a col given a byte index
int bline_get_col(bline_t *self, bint_t index, bint_t *ret_col) {
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
int bline_get_col_from_vcol(bline_t *bline, bint_t vcol, bint_t *ret_col) {
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

int bline_index_to_col(bline_t *bline, bint_t index, bint_t *ret_col) {
    *ret_col = _buffer_bline_index_to_col(bline, index);
    return MLBUF_OK;
}

static int _buffer_open_mmap(buffer_t *self, int fd, size_t size) {
    char tmppath[16];
    int tmpfd;
    char readbuf[1024];
    ssize_t nread;
    char *mmap_buf;

    // Copy fd to tmp file
    sprintf(tmppath, "%s", "/tmp/mle-XXXXXX");
    tmpfd = mkstemp(tmppath);
    if (tmpfd < 0) {
        return MLBUF_ERR;
    }
    unlink(tmppath);
    while (1) {
        nread = read(fd, &readbuf, 1024);
        if (nread == 0) {
            break;
        } else if (nread < 0) {
            close(tmpfd);
            return MLBUF_ERR;
        }
        if (write(tmpfd, readbuf, nread) != nread) {
            close(tmpfd);
            return MLBUF_ERR;
        }
    }

    // Now mmap tmp file
    mmap_buf = mmap(NULL, size, PROT_READ, MAP_PRIVATE, tmpfd, 0);
    if (mmap_buf == MAP_FAILED) {
        return MLBUF_ERR;
    } else if (buffer_set_mmapped(self, mmap_buf, (bint_t)size) != MLBUF_OK) {
        return MLBUF_ERR;
    }

    _buffer_munmap(self);
    self->mmap = mmap_buf;
    self->mmap_len = size;
    self->mmap_fd = tmpfd;
    return MLBUF_OK;
}

static int _buffer_open_read(buffer_t *self, int fd, size_t size) {
    int rc;
    char *buf;
    if (size <= PTRDIFF_MAX) {
        buf = malloc(size);
    } else {
        return MLBUF_ERR;
    }
    rc = MLBUF_OK;
    if (size != (size_t)read(fd, buf, size)) {
        rc = MLBUF_ERR;
    } else if (buffer_set(self, buf, (bint_t)size) != MLBUF_OK) {
        rc = MLBUF_ERR;
    }
    free(buf);
    return rc;
}

static int _buffer_bline_unslab(bline_t *self) {
    char *data;
    bline_char_t *chars;
    if (!self->is_data_slabbed) {
        return MLBUF_ERR;
    }
    data = malloc(self->data_len);
    chars = malloc(self->data_len * sizeof(bline_char_t));
    memcpy(data, self->data, self->data_len);
    memcpy(chars, self->chars, self->data_len * sizeof(bline_char_t));
    self->data = data;
    self->data_cap = self->data_len;
    self->chars = chars;
    self->chars_cap = self->data_len;
    self->is_data_slabbed = 0;
    return bline_count_chars(self);
}

static void _buffer_stat(buffer_t *self) {
    if (!self->path) {
        return;
    }
    stat(self->path, &self->st); // TODO err?
}

static int _buffer_baction_do(buffer_t *self, bline_t *bline, baction_t *action, int is_redo, bint_t *opt_repeat_offset) {
    int rc;
    bint_t col;
    bint_t offset;
    self->_is_in_undo = 1;
    col = opt_repeat_offset ? *opt_repeat_offset : action->start_col;
    buffer_get_offset(self, bline, col, &offset);
    if ((action->type == MLBUF_BACTION_TYPE_DELETE && is_redo)
        || (action->type == MLBUF_BACTION_TYPE_INSERT && !is_redo)
    ) {
        rc = buffer_delete(self, offset, (bint_t)((is_redo ? -1 : 1) * action->char_delta));
    } else {
        rc = buffer_insert(self, offset, action->data, action->data_len, NULL);
    }
    self->_is_in_undo = 0;
    return rc;
}

static int _buffer_update(buffer_t *self, baction_t *action) {
    bline_t *tmp_line;
    bline_t *last_line;
    bint_t new_line_index;

    // Adjust counts
    self->byte_count += action->byte_delta;
    self->line_count += action->line_delta;
    self->is_data_dirty = 1;

    // Set unsaved
    self->is_unsaved = 1;

    // Renumber lines
    if (action->line_delta != 0) {
        last_line = NULL;
        new_line_index = action->start_line->line_index;
        for (tmp_line = action->start_line->next; tmp_line != NULL; tmp_line = tmp_line->next) {
            tmp_line->line_index = ++new_line_index;
            last_line = tmp_line;
        }
        self->last_line = last_line ? last_line : action->start_line;
    }

    // Restyle from start_line
    buffer_apply_styles(self, action->start_line, action->line_delta);

    // Raise event on listener
    if (self->callback && !self->is_in_callback) {
        self->is_in_callback = 1;
        self->callback(self, action, self->callback_udata);
        self->is_in_callback = 0;
    }

    // Handle undo stack
    if (self->_is_in_undo) {
        _baction_destroy(action);
    } else {
        _buffer_add_to_undo_stack(self, action);
    }

    return MLBUF_OK;
}

static int _buffer_undo(buffer_t *self, int by_group) {
    baction_t *action_to_undo;
    baction_t *first_action;
    bline_t *bline;
    int group_to_undo;

    // Find action to undo
    if (self->action_undone) {
        if (self->action_undone == self->actions) {
            return MLBUF_ERR;
        } else if (!self->action_undone->prev) {
            return MLBUF_ERR;
        }
        action_to_undo = self->action_undone->prev;
    } else if (self->action_tail) {
        action_to_undo = self->action_tail;
    } else {
        return MLBUF_ERR;
    }
    first_action = action_to_undo;

    // Remember action group for coarse undo (by_group)
    group_to_undo = action_to_undo->action_group;

    while (1) {
        // Get line to perform undo on
        bline = NULL;
        buffer_get_bline(self, action_to_undo->start_line_index, &bline);
        MLBUF_BLINE_ENSURE_CHARS(bline);
        if (!bline) {
            return MLBUF_ERR;
        } else if (action_to_undo->start_col > bline->char_count) {
            return MLBUF_ERR;
        }

        // Perform action
        if (_buffer_baction_do(self, bline, action_to_undo, 0, NULL) != MLBUF_OK) {
            return MLBUF_ERR;
        }
        self->action_undone = action_to_undo;

        // If by_group, undo next action in same group or break
        if (by_group
            && action_to_undo->prev
            && action_to_undo->prev != first_action
            && action_to_undo->prev->action_group == group_to_undo
        ) {
            action_to_undo = action_to_undo->prev;
        } else {
            break;
        }
    }

    return MLBUF_OK;
}

static int _buffer_redo(buffer_t *self, int by_group) {
    baction_t *action_to_redo;
    bline_t *bline;
    int *group_to_redo;

    // Find action to undo
    if (!self->action_undone) {
        return MLBUF_ERR;
    }
    action_to_redo = self->action_undone;

    // Set action group
    group_to_redo = by_group ? &action_to_redo->action_group : NULL;

    while (1) {
        // Get line to perform undo on
        bline = NULL;
        buffer_get_bline(self, action_to_redo->start_line_index, &bline);
        MLBUF_BLINE_ENSURE_CHARS(bline);
        if (!bline) {
            return MLBUF_ERR;
        } else if (action_to_redo->start_col > bline->char_count) {
            return MLBUF_ERR;
        }

        // Perform action
        if (_buffer_baction_do(self, bline, action_to_redo, 1, NULL) != MLBUF_OK) {
            return MLBUF_ERR;
        }
        self->action_undone = action_to_redo->next;

        // Redo next action in same group or break
        if (group_to_redo
            && action_to_redo->next
            && action_to_redo->next != action_to_redo
            && action_to_redo->next->action_group == *group_to_redo
        ) {
            action_to_redo = action_to_redo->next;
        } else {
            break;
        }
    }

    return MLBUF_OK;
}

static int _buffer_truncate_undo_stack(buffer_t *self, baction_t *action_from) {
    baction_t *action_target;
    baction_t *action_tmp;
    int do_delete;
    self->action_tail = action_from->prev != action_from ? action_from->prev : NULL;
    do_delete = 0;
    DL_FOREACH_SAFE(self->actions, action_target, action_tmp) {
        if (!do_delete && action_target == action_from) {
            do_delete = 1;
        }
        if (do_delete) {
            DL_DELETE(self->actions, action_target);
            _baction_destroy(action_target);
        }
    }
    return MLBUF_OK;
}

static int _buffer_add_to_undo_stack(buffer_t *self, baction_t *action) {
    if (self->action_undone) {
        // We are recording an action after an undo has been performed, so we
        // need to chop off the tail of the baction list before recording the
        // new one.
        // TODO could implement multilevel undo here instead
        _buffer_truncate_undo_stack(self, self->action_undone);
        self->action_undone = NULL;
    }

    // Append action to list
    DL_APPEND(self->actions, action);
    if (self->action_group) action->action_group = *self->action_group;
    self->action_tail = action;
    return MLBUF_OK;
}

static int _buffer_apply_styles_all(bline_t *bline, bint_t min_nlines) {
    buffer_t *buffer;
    srule_node_t *srule_node;
    srule_t *open_rule, *found_rule, *eol_rule_orig;
    bint_t col, styled_nlines, start, stop;
    int eol_rule_changed;

    buffer = bline->buffer;
    open_rule = bline->prev ? bline->prev->eol_rule : NULL;
    styled_nlines = 0;
    col = 0;

    _buffer_bline_reset_styles(bline);

    while (1) {
        found_rule = NULL;

        // Reset style cache at beginning of each line
        if (col == 0) {
            DL_FOREACH(buffer->srules, srule_node) {
                memset(&srule_node->srule->memo, 0, sizeof(srule_node->srule->memo));
                memset(&srule_node->srule->memo_end, 0, sizeof(srule_node->srule->memo_end));
            }
        }

        if (bline->char_count <= 0) {
            // Nothing to do on empty line
        } else if (open_rule) {
            // Look for end of open_rule
            if (_buffer_match_srule(bline, col, open_rule, 1, &start, &stop)) {
                // End of open_rule found; close rule
                found_rule = open_rule;
                open_rule = NULL;
            } else {
                // End of open_rule NOT found; style entire line
                found_rule = open_rule;
                start = col;
                stop = bline->char_count;
            }
        } else {
            // Look for any rule
            DL_FOREACH(buffer->srules, srule_node) {
                if (_buffer_match_srule(bline, col, srule_node->srule, 0, &start, &stop) && start == col) {
                    found_rule = srule_node->srule;
                    if (srule_node->srule->cre_end) open_rule = srule_node->srule;
                    break;
                }
            }
        }

        if (found_rule) {
            // Set style of found_rule
            _buffer_bline_style(bline, MLBUF_MIN(start, col), stop, &found_rule->style);
            col = MLBUF_MAX(stop, col + 1);
        } else {
            // Nothing found; advance one char
            col += 1;
        }

        if (col >= bline->char_count) {
            // At end of line

            // Set eol_rule and already_open
            eol_rule_orig = bline->eol_rule;
            bline->eol_rule = open_rule; // can be NULL
            eol_rule_changed = eol_rule_orig != bline->eol_rule ? 1 : 0;

            // Advance to next line
            styled_nlines += 1;
            bline = bline->next;
            col = 0;

            // Check stop conditions
            if (!bline || (styled_nlines >= min_nlines && !eol_rule_changed)) {
                break;
            }

            // Clear style of next line
            _buffer_bline_reset_styles(bline);
        }
    }

    return MLBUF_OK;
}

static int _buffer_match_srule(bline_t *bline, bint_t look_offset, srule_t *srule, int end_rule, bint_t *ret_start, bint_t *ret_stop) {
    int rc;
    PCRE2_SIZE substrs[3];
    pcre2_code *cre;
    smemo_t *memo;

    cre = end_rule ? srule->cre_end : srule->cre;
    memo = end_rule ? &srule->memo_end : &srule->memo;

    if (memo->looked && look_offset >= memo->look_offset) {
        if (memo->found) {
            if (look_offset <= memo->start) {
                *ret_start = memo->start;
                *ret_stop = memo->stop;
                return 1;
            }
        } else {
            return 0;
        }
    }

    rc = pcre2_match(cre, (PCRE2_SPTR)bline->data, (PCRE2_SIZE)bline->data_len, (PCRE2_SIZE)look_offset, 0, pcre2_md, NULL);
    memo->looked = 1;
    memo->look_offset = look_offset;
    memo->found = 0;
    if (rc < 0) return 0;

    memcpy(substrs, pcre2_get_ovector_pointer(pcre2_md), 3 * sizeof(PCRE2_SIZE));
    if (substrs[1] == PCRE2_UNSET) return 0;

    *ret_start = _buffer_bline_index_to_col(bline, substrs[0]);
    *ret_stop = _buffer_bline_index_to_col(bline, substrs[1]);

    memo->found = 1;
    memo->start = *ret_start;
    memo->stop = *ret_stop;

    return 1;
}

static void _buffer_bline_reset_styles(bline_t *bline) {
    sblock_t reset = {0};
    _buffer_bline_style(bline, 0, bline->char_count, &reset);
}

static void _buffer_bline_style(bline_t *bline, bint_t start, bint_t stop, sblock_t *style) {
    bint_t i;
    for (i = start; i < stop && i < bline->char_count; i++) {
        bline->chars[i].style = *style;
    }
}

static bline_t *_buffer_bline_new(buffer_t *self) {
    bline_t *bline;
    bline = calloc(1, sizeof(bline_t));
    bline->buffer = self;
    return bline;
}

static int _buffer_bline_free(bline_t *bline, bline_t *maybe_mark_line, bint_t col_delta) {
    mark_t *mark;
    mark_t *mark_tmp;
    if (!bline->is_data_slabbed) {
        if (bline->data) free(bline->data);
        if (bline->chars) free(bline->chars);
    }
    if (bline->marks) {
        DL_FOREACH_SAFE(bline->marks, mark, mark_tmp) {
            if (maybe_mark_line) {
                _mark_mark_move_inner(mark, maybe_mark_line, mark->col + col_delta, 1);
            } else {
                buffer_destroy_mark(bline->buffer, mark);
            }
        }
    }
    if (!bline->is_slabbed) {
        free(bline);
    }
    return MLBUF_OK;
}

static bline_t *_buffer_bline_break(bline_t *bline, bint_t col) {
    bint_t index;
    bint_t len;
    bline_t *new_line;
    bline_t *tmp_line;
    mark_t *mark;
    mark_t *mark_tmp;

    // Unslab if needed
    if (bline->is_data_slabbed) _buffer_bline_unslab(bline);

    // Make new_line
    new_line = _buffer_bline_new(bline->buffer);

    // Find byte index to break on
    index = _buffer_bline_col_to_index(bline, col);
    len = bline->data_len - index;

    if (len > 0) {
        // Move data to new line
        new_line->data = malloc(len);
        memcpy(new_line->data, bline->data + index, len);
        new_line->data_len = len;
        new_line->data_cap = len;
        bline_count_chars(new_line); // Update char widths

        // Truncate orig line
        bline->data_len -= len;
        bline_count_chars(bline); // Update char widths
    }

    // Insert new_line in linked list
    tmp_line = bline->next;
    bline->next = new_line;
    new_line->next = tmp_line;
    new_line->prev = bline;
    if (tmp_line) tmp_line->prev = new_line;

    // Move marks at or past col to new_line
    DL_FOREACH_SAFE(bline->marks, mark, mark_tmp) {
        if (mark_is_after_col_minus_lefties(mark, col)) {
            _mark_mark_move_inner(mark, new_line, mark->col - col, 1);
        }
    }

    return new_line;
}

// Given start_line:start_col + num_chars, find end_line:end_col
static void _buffer_find_end_pos(bline_t *start_line, bint_t start_col, bint_t num_chars, bline_t **ret_end_line, bint_t *ret_end_col, bint_t *ret_safe_num_chars) {
    bline_t *end_line;
    bint_t end_col;
    bint_t num_chars_rem;
    end_line = start_line;
    end_col = start_col;
    num_chars_rem = num_chars;
    while (num_chars_rem > 0) {
        MLBUF_BLINE_ENSURE_CHARS(end_line);
        if (end_line->char_count - end_col >= num_chars_rem) {
            end_col += num_chars_rem;
            num_chars_rem = 0;
        } else {
            num_chars_rem -= (end_line->char_count - end_col) + 1;
            if (end_line->next) {
                end_line = end_line->next;
                end_col = 0;
            } else {
                end_col = end_line->char_count;
                break;
            }
        }
    }
    num_chars -= num_chars_rem;
    *ret_end_line = end_line;
    *ret_end_col = end_col;
    *ret_safe_num_chars = num_chars;
}

static void _buffer_bline_replace(bline_t *bline, bint_t start_col, char *data, bint_t data_len, str_t *del_data) {
    bint_t start_index;
    mark_t *mark;

    // Unslab if needed
    if (bline->is_data_slabbed) _buffer_bline_unslab(bline);

    // Realloc if needed
    MLBUF_BLINE_ENSURE_CHARS(bline);
    if (start_col <= 0) {
        start_index = 0;
    } else if (start_col >= bline->char_count) {
        start_index = bline->data_len;
    } else {
        start_index = bline->chars[start_col].index;
    }
    if (start_index + data_len >= bline->data_cap) {
        bline->data = realloc(bline->data, start_index + data_len);
        bline->data_cap = start_index + data_len;
    }

    // Store del_data
    str_append_len(del_data, bline->data + start_index, bline->data_len - start_index);

    // Copy data into slot and update chars
    memmove(bline->data + start_index, data, (size_t)data_len);
    bline->data_len = start_index + data_len;
    bline_count_chars(bline);

    // Fix marks
    DL_FOREACH(bline->marks, mark) {
        if (mark->col > bline->char_count) {
            mark->col = bline->char_count;
        }
    }
}

static bint_t _buffer_bline_insert(bline_t *bline, bint_t col, char *data, bint_t data_len, int move_marks) {
    bint_t index;
    mark_t *mark;
    mark_t *mark_tmp;
    bint_t orig_char_count;
    bint_t num_chars_added;

    // Unslab if needed
    if (bline->is_data_slabbed) _buffer_bline_unslab(bline);

    // Get orig char_count
    MLBUF_BLINE_ENSURE_CHARS(bline);
    orig_char_count = bline->char_count;

    // Ensure space for data
    if (!bline->data) {
        bline->data = malloc(data_len);
        bline->data_cap = data_len;
    } else if (bline->data_len + data_len > bline->data_cap) {
        bline->data = realloc(bline->data, bline->data_len + data_len);
        bline->data_cap = bline->data_len + data_len;
    }

    // Find insert point
    index = _buffer_bline_col_to_index(bline, col);

    // Make room for insert data
    if (index < bline->data_len) {
        memmove(bline->data + index + data_len, bline->data + index, bline->data_len - index);
    }
    bline->data_len += data_len;

    // Insert data
    memcpy(bline->data + index, data, data_len);

    // Update chars
    bline_count_chars(bline);
    num_chars_added = bline->char_count - orig_char_count;

    // Move marks after col right by num_chars_added
    if (move_marks) {
        DL_FOREACH_SAFE(bline->marks, mark, mark_tmp) {
            if (mark_is_after_col_minus_lefties(mark, col)) {
                mark->col += num_chars_added;
            }
        }
    }

    return num_chars_added;
}

static bint_t _buffer_bline_delete(bline_t *bline, bint_t col, bint_t num_chars) {
    bint_t safe_num_chars;
    bint_t index;
    bint_t index_end;
    bint_t move_len;
    mark_t *mark;
    mark_t *mark_tmp;
    bint_t orig_char_count;
    bint_t num_chars_deleted;

    // Unslab if needed
    if (bline->is_data_slabbed) _buffer_bline_unslab(bline);

    // Get orig char_count
    MLBUF_BLINE_ENSURE_CHARS(bline);
    orig_char_count = bline->char_count;

    // Clamp num_chars
    safe_num_chars = MLBUF_MIN(bline->char_count - col, num_chars);
    if (safe_num_chars != num_chars) {
        MLBUF_DEBUG_PRINTF("num_chars=%" PRIdMAX " does not match safe_num_chars=%" PRIdMAX "\n", num_chars, safe_num_chars);
    }

    // Nothing to do if safe_num_chars is 0
    if (safe_num_chars < 1) {
        MLBUF_DEBUG_PRINTF("safe_num_chars=%" PRIdMAX " lt 1\n", safe_num_chars);
        return MLBUF_OK;
    }

    // Find delete bounds
    index = _buffer_bline_col_to_index(bline, col);
    index_end = _buffer_bline_col_to_index(bline, col + safe_num_chars);
    move_len = (bint_t)(bline->data_len - index_end);

    // Shift data
    if (move_len > 0) {
        memmove(bline->data + index, bline->data + index_end, move_len);
    }
    bline->data_len -= index_end - index;

    // Update chars
    bline_count_chars(bline);
    num_chars_deleted = orig_char_count - bline->char_count;

    // Move marks after col left by num_chars_deleted
    DL_FOREACH_SAFE(bline->marks, mark, mark_tmp) {
        if (mark->col > col) {
            mark->col = MLBUF_MAX(0, mark->col - num_chars_deleted);
        }
    }

    return num_chars_deleted;
}

static bint_t _buffer_bline_col_to_index(bline_t *bline, bint_t col) {
    bint_t index;
    MLBUF_BLINE_ENSURE_CHARS(bline);
    if (!bline->chars) {
        return 0;
    }
    if (col >= bline->char_count) {
        index = bline->data_len;
    } else {
        index = bline->chars[col].index;
    }
    return index;
}

static bint_t _buffer_bline_index_to_col(bline_t *bline, bint_t index) {
    MLBUF_BLINE_ENSURE_CHARS(bline);
    if (index < 1) {
        return 0;
    } else if (index >= bline->data_len) {
        return bline->char_count;
    }
    return bline->chars[index].index_to_vcol;
}

// Close self->fd and self->mmap if needed
static int _buffer_munmap(buffer_t *self) {
    if (self->mmap) {
        munmap(self->mmap, self->mmap_len);
        close(self->mmap_fd);
        self->mmap = NULL;
        self->mmap_len = 0;
        self->mmap_fd = -1;
    }
    return MLBUF_OK;
}

// Make a new single-line style rule
srule_t *srule_new_single(char *re, bint_t re_len, int caseless, uint16_t fg, uint16_t bg) {
    srule_t *rule;
    int re_errcode;
    PCRE2_SIZE re_erroffset;
    rule = calloc(1, sizeof(srule_t));
    rule->type = MLBUF_SRULE_TYPE_SINGLE;
    rule->style.fg = fg;
    rule->style.bg = bg;
    rule->re = malloc((re_len + 1) * sizeof(char));
    snprintf(rule->re, re_len + 1, "%.*s", (int)re_len, re);
    rule->cre = pcre2_compile((PCRE2_SPTR)rule->re, (PCRE2_SIZE)strlen(rule->re), PCRE2_NO_AUTO_CAPTURE | (caseless ? PCRE2_CASELESS : 0), &re_errcode, &re_erroffset, NULL);
    if (!rule->cre) {
        // TODO log error
        srule_destroy(rule);
        return NULL;
    }
    return rule;
}

// Make a new multi-line style rule
srule_t *srule_new_multi(char *re, bint_t re_len, char *re_end, bint_t re_end_len, uint16_t fg, uint16_t bg) {
    srule_t *rule;
    int re_errcode;;
    PCRE2_SIZE re_erroffset;
    rule = calloc(1, sizeof(srule_t));
    rule->type = MLBUF_SRULE_TYPE_MULTI;
    rule->style.fg = fg;
    rule->style.bg = bg;
    rule->re = malloc((re_len + 1) * sizeof(char));
    rule->re_end = malloc((re_end_len + 1) * sizeof(char));
    snprintf(rule->re, re_len + 1, "%.*s", (int)re_len, re);
    snprintf(rule->re_end, re_end_len + 1, "%.*s", (int)re_end_len, re_end);
    rule->cre = pcre2_compile((PCRE2_SPTR)rule->re, (PCRE2_SIZE)strlen(rule->re), PCRE2_NO_AUTO_CAPTURE, &re_errcode, &re_erroffset, NULL);
    rule->cre_end = pcre2_compile((PCRE2_SPTR)rule->re_end, (PCRE2_SIZE)strlen(rule->re_end), PCRE2_NO_AUTO_CAPTURE, &re_errcode, &re_erroffset, NULL);
    if (!rule->cre || !rule->cre_end) {
        // TODO log error
        srule_destroy(rule);
        return NULL;
    }
    return rule;
}

// Make a new range style rule
srule_t *srule_new_range(mark_t *range_a, mark_t *range_b, uint16_t fg, uint16_t bg) {
    srule_t *rule;
    rule = calloc(1, sizeof(srule_t));
    rule->type = MLBUF_SRULE_TYPE_RANGE;
    rule->style.fg = fg;
    rule->style.bg = bg;
    rule->range_a = range_a;
    rule->range_b = range_b;
    return rule;
}

// Free an srule
int srule_destroy(srule_t *srule) {
    if (srule->re) free(srule->re);
    if (srule->re_end) free(srule->re_end);
    if (srule->cre) pcre2_code_free(srule->cre);
    if (srule->cre_end) pcre2_code_free(srule->cre_end);
    free(srule);
    return MLBUF_OK;
}

static int _baction_destroy(baction_t *action) {
    if (action->data) free(action->data);
    free(action);
    return MLBUF_OK;
}

// Move self/col forward until col fits on current line
static void _bline_advance_col(bline_t **self, bint_t *col) {
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
