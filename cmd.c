#include "mle.h"

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
    editor_prompt(ctx->editor, "cmd_move_to_line", "Line?", &linestr);
    line = strtoll(linestr, NULL, 10);
    if (line < 1) line = 1;
    MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_move_to, line - 1, 0);
    bview_center_viewport_y(ctx->bview);
    return MLE_OK;
}

int cmd_move_word_forward(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_move_word_back(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_anchor_sel_bound(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_search(cmd_context_t* ctx) {
    return MLE_OK;
}
int cmd_search_next(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_replace(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_isearch(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_delete_word_before(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_delete_word_after(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_cut(cmd_context_t* ctx) {
return MLE_OK; }
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

// Split a bview
int cmd_split(cmd_context_t* ctx) {
    bview_t* child;
    if (bview_split(ctx->bview, 1, 0.5, &child) == MLE_OK) {
        editor_set_active(ctx->editor, child);
    }
    return MLE_OK;
}

int cmd_save(cmd_context_t* ctx) {
    return MLE_OK;
}

int cmd_open(cmd_context_t* ctx) {
    return MLE_OK;
}

int cmd_close(cmd_context_t* ctx) {
    editor_close_bview(ctx->editor, ctx->bview);
    return MLE_OK;
}

// Quit
int cmd_quit(cmd_context_t* ctx) {
    ctx->loop_ctx->should_exit = 1; // TODO prompt for changes
    return MLE_OK;
}
