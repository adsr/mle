#include "mle.h"

static int _cmd_pre_close(editor_t* editor, bview_t* bview);
static int _cmd_quit_inner(editor_t* editor, bview_t* bview);
static int _cmd_save(editor_t* editor, bview_t* bview);

// Insert data
int cmd_insert_data(cmd_context_t* ctx) {
    char data[6];
    bint_t data_len;
    if (ctx->input.ch) {
        data_len = utf8_unicode_to_char(data, ctx->input.ch);
    } else if (ctx->input.key == TB_KEY_ENTER || ctx->input.key == TB_KEY_CTRL_J) {
        data_len = sprintf(data, "\n");
    } else if (ctx->input.key >= 0x20 && ctx->input.key <= 0x7e) {
        data_len = sprintf(data, "%c", ctx->input.key);
    } else if (ctx->input.key == 0x09) {
        data_len = sprintf(data, "\t");
    } else {
        data_len = 0;
    }
    if (data_len < 1) {
        return MLE_OK;
    }
    MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_insert_before, data, data_len);
    return MLE_OK;
}

// Insert newline
int cmd_insert_newline(cmd_context_t* ctx) {
    return cmd_insert_data(ctx);
}

// Insert tab
int cmd_insert_tab(cmd_context_t* ctx) {
    int num_spaces;
    char* data;
    if (ctx->bview->tab_to_space) {
        num_spaces = ctx->bview->buffer->tab_width - (ctx->cursor->mark->col % ctx->bview->buffer->tab_width);
        data = malloc(num_spaces);
        memset(data, ' ', num_spaces);
        MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_insert_before, data, (bint_t)num_spaces);
        free(data);
    } else {
        MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_insert_before, (char*)"\t", (bint_t)1);
    }
    return MLE_OK;
}

// Delete char before cursor mark
int cmd_delete_before(cmd_context_t* ctx) {
    bint_t offset;
    mark_get_offset(ctx->cursor->mark, &offset);
    if (offset < 1) return MLE_OK;
    MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_delete_before, 1);
    return MLE_OK;
}

// Delete char after cursor mark
int cmd_delete_after(cmd_context_t* ctx) {
    MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_delete_after, 1);
    return MLE_OK;
}

// Move cursor to beginning of line
int cmd_move_bol(cmd_context_t* ctx) {
    MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_move_bol);
    bview_rectify_viewport(ctx->bview);
    return MLE_OK;
}

// Move cursor to end of line
int cmd_move_eol(cmd_context_t* ctx) {
    MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_move_eol);
    bview_rectify_viewport(ctx->bview);
    return MLE_OK;
}

// Move cursor to beginning of buffer
int cmd_move_beginning(cmd_context_t* ctx) {
    MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_move_beginning);
    bview_rectify_viewport(ctx->bview);
    return MLE_OK;
}

// Move cursor to end of buffer
int cmd_move_end(cmd_context_t* ctx) {
    MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_move_end);
    bview_rectify_viewport(ctx->bview);
    return MLE_OK;
}

// Move cursor left one char
int cmd_move_left(cmd_context_t* ctx) {
    MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_move_by, -1);
    bview_rectify_viewport(ctx->bview);
    return MLE_OK;
}

// Move cursor right one char
int cmd_move_right(cmd_context_t* ctx) {
    MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_move_by, 1);
    bview_rectify_viewport(ctx->bview);
    return MLE_OK;
}

// Move cursor up one line
int cmd_move_up(cmd_context_t* ctx) {
    MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_move_vert, -1);
    bview_rectify_viewport(ctx->bview);
    return MLE_OK;
}

// Move cursor down one line
int cmd_move_down(cmd_context_t* ctx) {
    MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_move_vert, 1);
    bview_rectify_viewport(ctx->bview);
    return MLE_OK;
}

// Move cursor one page up
int cmd_move_page_up(cmd_context_t* ctx) {
    MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_move_vert, -1 * ctx->bview->rect_buffer.h);
    bview_zero_viewport_y(ctx->bview);
    return MLE_OK;
}

// Move cursor one page down
int cmd_move_page_down(cmd_context_t* ctx) {
    MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_move_vert, ctx->bview->rect_buffer.h);
    bview_zero_viewport_y(ctx->bview);
    return MLE_OK;
}

// Move to specific line
int cmd_move_to_line(cmd_context_t* ctx) {
    char* linestr;
    bint_t line;
    editor_prompt(ctx->editor, "cmd_move_to_line", "Line?", NULL, 0, NULL, &linestr);
    if (!linestr) return MLE_OK;
    line = strtoll(linestr, NULL, 10);
    free(linestr);
    if (line < 1) line = 1;
    MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_move_to, line - 1, 0);
    bview_center_viewport_y(ctx->bview);
    return MLE_OK;
}

// Move one word forward
int cmd_move_word_forward(cmd_context_t* ctx) {
    MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_move_next_re, "((?<=\\w)\\W|$)", sizeof("((?<=\\w)\\W|$)")-1);
    return MLE_OK;
}

// Move one word back
int cmd_move_word_back(cmd_context_t* ctx) {
    MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_move_prev_re, "((?<=\\W)\\w|^)", sizeof("((?<=\\W)\\w|^)")-1);
    return MLE_OK;
}

// Delete word back
int cmd_delete_word_before(cmd_context_t* ctx) {
    cursor_t* cursor;
    mark_t* tmark;
    DL_FOREACH(ctx->bview->cursors, cursor) {
        if (cursor->is_asleep) continue;
        tmark = mark_clone(cursor->mark);
        mark_move_prev_re(tmark, "((?<=\\W)\\w|^)", sizeof("((?<=\\W)\\w|^)")-1);
        mark_delete_between_mark(cursor->mark, tmark);
        mark_destroy(tmark);
    }
    return MLE_OK;
}

// Delete word ahead
int cmd_delete_word_after(cmd_context_t* ctx) {
    cursor_t* cursor;
    mark_t* tmark;
    DL_FOREACH(ctx->bview->cursors, cursor) {
        if (cursor->is_asleep) continue;
        tmark = mark_clone(cursor->mark);
        mark_move_next_re(tmark, "((?<=\\w)\\W|$)", sizeof("((?<=\\w)\\W|$)")-1);
        mark_delete_between_mark(cursor->mark, tmark);
        mark_destroy(tmark);
    }
    return MLE_OK;
}

// Toggle sel bound on cursors
int cmd_toggle_sel_bound(cmd_context_t* ctx) {
    cursor_t* cursor;
    DL_FOREACH(ctx->bview->cursors, cursor) {
        if (!cursor->is_sel_bound_anchored) {
            cursor->sel_bound = mark_clone(cursor->mark);
            cursor->sel_rule = srule_new_range(cursor->mark, cursor->sel_bound, 0, TB_REVERSE);
            buffer_add_srule(ctx->bview->buffer, cursor->sel_rule);
            cursor->is_sel_bound_anchored = 1;
        } else {
            buffer_remove_srule(ctx->bview->buffer, cursor->sel_rule);
            srule_destroy(cursor->sel_rule);
            cursor->sel_rule = NULL;
            mark_destroy(cursor->sel_bound);
            cursor->is_sel_bound_anchored = 0;
        }
    }
    return MLE_OK;
}

// Drop an is_asleep=1 cursor
int cmd_drop_sleeping_cursor(cmd_context_t* ctx) {
    cursor_t* cursor;
    bview_add_cursor(ctx->bview, ctx->cursor->mark->bline, ctx->cursor->mark->col, &cursor);
    cursor->is_asleep = 1;
    return MLE_OK;
}

// Awake all is_asleep=1 cursors
int cmd_wake_sleeping_cursors(cmd_context_t* ctx) {
    cursor_t* cursor;
    DL_FOREACH(ctx->bview->cursors, cursor) {
        if (cursor->is_asleep) {
            cursor->is_asleep = 0;
        }
    }
    return MLE_OK;
}

// Remove all cursors except the active one
int cmd_remove_extra_cursors(cmd_context_t* ctx) {
    cursor_t* cursor;
    DL_FOREACH(ctx->bview->cursors, cursor) {
        if (cursor != ctx->cursor) {
            bview_remove_cursor(ctx->bview, cursor);
        }
    }
    return MLE_OK;
}

int cmd_search(cmd_context_t* ctx) {
    return MLE_OK;
}
int cmd_search_next(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_replace(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_isearch(cmd_context_t* ctx) {
return MLE_OK; }

int cmd_cut(cmd_context_t* ctx) {
    return MLE_OK;
}

int cmd_uncut(cmd_context_t* ctx) {
return MLE_OK; }

// Switch focus to next/prev bview (cmd_next, cmd_prev)
#define MLE_IMPL_CMD_NEXTPREV(pthis, pend) \
int cmd_ ## pthis (cmd_context_t* ctx) { \
    bview_t* tmp; \
    for (tmp = ctx->bview->pthis; tmp != ctx->bview; tmp = tmp->pthis) { \
        if (tmp == NULL) { \
            tmp = (pend); \
            if (tmp == NULL) break; \
        } \
        if (MLE_BVIEW_IS_EDIT(tmp)) { \
            editor_set_active(ctx->editor, tmp); \
            break; \
        } \
    } \
    return MLE_OK; \
}
MLE_IMPL_CMD_NEXTPREV(next, ctx->editor->bviews)
MLE_IMPL_CMD_NEXTPREV(prev, ctx->editor->bviews_tail)
#undef MLE_IMPL_CMD_NEXTPREV

// Split a bview vertically
int cmd_split_vertical(cmd_context_t* ctx) {
    bview_t* child;
    if (bview_split(ctx->bview, 1, 0.5, &child) == MLE_OK) {
        editor_set_active(ctx->editor, child);
    }
    return MLE_OK;
}

// Split a bview horizontally
int cmd_split_horizontal(cmd_context_t* ctx) {
    bview_t* child;
    if (bview_split(ctx->bview, 0, 0.5, &child) == MLE_OK) {
        editor_set_active(ctx->editor, child);
    }
    return MLE_OK;
}

// Save file
int cmd_save(cmd_context_t* ctx) {
    _cmd_save(ctx->editor, ctx->bview);
    return MLE_OK;
}

// Open file in a new bview
int cmd_new_open(cmd_context_t* ctx) {
    char* path;
    editor_prompt(ctx->editor, "cmd_new_open", "File?", NULL, 0, NULL, &path);
    if (!path) return MLE_OK;
    editor_open_bview(ctx->editor, MLE_BVIEW_TYPE_EDIT, path, strlen(path), 1, &ctx->editor->rect_edit, NULL, NULL);
    free(path);
    return MLE_OK;
}

// Open empty buffer in a new bview
int cmd_new(cmd_context_t* ctx) {
    editor_open_bview(ctx->editor, MLE_BVIEW_TYPE_EDIT, NULL, 0, 1, &ctx->editor->rect_edit, NULL, NULL);
    return MLE_OK;
}

// Open file into current bview
int cmd_replace_open(cmd_context_t* ctx) {
    char* path;
    if (!_cmd_pre_close(ctx->editor, ctx->bview)) return MLE_OK;
    path = NULL;
    editor_prompt(ctx->editor, "cmd_replace_open", "Replace with file?", NULL, 0, NULL, &path);
    if (!path) return MLE_OK;
    bview_open(ctx->bview, path, strlen(path));
    bview_resize(ctx->bview, ctx->bview->x, ctx->bview->y, ctx->bview->w, ctx->bview->h);
    free(path);
    return MLE_OK;
}

// Open empty buffer into current bview
int cmd_replace_new(cmd_context_t* ctx) {
    if (!_cmd_pre_close(ctx->editor, ctx->bview)) return MLE_OK;
    bview_open(ctx->bview, NULL, 0);
    bview_resize(ctx->bview, ctx->bview->x, ctx->bview->y, ctx->bview->w, ctx->bview->h);
    return MLE_OK;
}

// Close bview
int cmd_close(cmd_context_t* ctx) {
    int should_exit;
    if (!_cmd_pre_close(ctx->editor, ctx->bview)) return MLE_OK;
    should_exit = editor_bview_edit_count(ctx->editor) <= 1 ? 1 : 0;
    editor_close_bview(ctx->editor, ctx->bview);
    ctx->loop_ctx->should_exit = should_exit;
    return MLE_OK;
}

// Quit editor
int cmd_quit(cmd_context_t* ctx) {
    bview_t* bview;
    bview_t* tmp;
    DL_FOREACH_SAFE(ctx->editor->bviews, bview, tmp) {
        if (!MLE_BVIEW_IS_EDIT(bview)) {
            continue;
        } else if (!_cmd_quit_inner(ctx->editor, bview)) {
            return MLE_OK;
        }
    }
    ctx->loop_ctx->should_exit = 1;
    return MLE_OK;
}

// Recursively close bviews, prompting to save unsaved changes.  Return 1 if
// it's OK to continue closing, or 0 if the action was cancelled.
static int _cmd_quit_inner(editor_t* editor, bview_t* bview) {
    if (bview->split_child && !_cmd_quit_inner(editor, bview->split_child)) {
        return 0;
    } else if (!_cmd_pre_close(editor, bview)) {
        return 0;
    }
    editor_close_bview(editor, bview);
    return 1;
}

// Prompt to save unsaved changes on close. Return 1 if it's OK to continue
// closing the bview, or 0 if the action was cancelled.
static int _cmd_pre_close(editor_t* editor, bview_t* bview) {
    char* yn;
    if (!bview->buffer->is_unsaved) return 1;

    editor_set_active(editor, bview);

    yn = NULL;
    editor_prompt(editor, "_cmd_pre_close", "Save modified? (y=yes, n=no, C-c=cancel)",
        NULL,
        0,
        editor->kmap_prompt_yn,
        &yn
    );
    if (!yn) {
        return 0;
    } else if (0 == strcmp(yn, MLE_PROMPT_NO)) {
        return 1;
    }

    return _cmd_save(editor, bview);
}

// Prompt to save changes. Return 1 if file was saved or 0 if the action was
// cancelled.
static int _cmd_save(editor_t* editor, bview_t* bview) {
    int rc;
    char* path;
    do {
        path = NULL;
        editor_prompt(editor, "_cmd_save", "Save as? (C-c=cancel)",
            bview->buffer->path,
            bview->buffer->path ? strlen(bview->buffer->path) : 0,
            NULL,
            &path
        );
        if (!path) return 0;
        rc = buffer_save_as(bview->buffer, path, strlen(path)); // TODO display error
        free(path);
    } while (rc == MLBUF_ERR);
    return 1;
}
