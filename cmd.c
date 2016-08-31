#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "mle.h"

#define MLE_MULTI_CURSOR_MARK_FN(pcursor, pfn, ...) do {\
    cursor_t* cursor; \
    DL_FOREACH((pcursor)->bview->cursors, cursor) { \
        if (cursor->is_asleep) continue; \
        pfn(cursor->mark, ##__VA_ARGS__); \
    } \
} while(0)

#define MLE_MULTI_CURSOR_CODE(pcursor, pcode) do { \
    cursor_t* cursor; \
    DL_FOREACH((pcursor)->bview->cursors, cursor) { \
        if (cursor->is_asleep) continue; \
        pcode \
    } \
} while(0)

static int _cmd_pre_close(editor_t* editor, bview_t* bview);
static int _cmd_quit_inner(editor_t* editor, bview_t* bview);
static int _cmd_save(editor_t* editor, bview_t* bview, int save_as);
static void _cmd_cut_copy(cursor_t* cursor, int is_cut, int use_srules, int append);
static void _cmd_toggle_anchor(cursor_t* cursor, int use_srules);
static int _cmd_search_next(bview_t* bview, cursor_t* cursor, mark_t* search_mark, char* regex, int regex_len);
static void _cmd_aproc_passthru_cb(async_proc_t* self, char* buf, size_t buf_len, int is_error, int is_eof, int is_timeout);
static void _cmd_fsearch_prompt_cb(bview_t* bview, baction_t* action, void* udata);
static void _cmd_isearch_prompt_cb(bview_t* bview, baction_t* action, void* udata);
static int _cmd_browse_cb(cmd_context_t* ctx);
static int _cmd_grep_cb(cmd_context_t* ctx);
static int _cmd_select_by(cursor_t* cursor, char* strat);
static int _cmd_select_by_bracket(cursor_t* cursor);
static int _cmd_select_by_word(cursor_t* cursor);
static int _cmd_select_by_word_back(cursor_t* cursor);
static int _cmd_select_by_word_forward(cursor_t* cursor);
static int _cmd_select_by_string(cursor_t* cursor);
static int _cmd_indent(cmd_context_t* ctx, int outdent);
static int _cmd_indent_line(bline_t* bline, int use_tabs, int outdent);
static void _cmd_help_inner(char* buf, kbinding_t* trie, char** h, int* l, int* s);

// Insert data
int cmd_insert_data(cmd_context_t* ctx) {
    bint_t insertbuf_len;
    char* insertbuf_cur;
    bint_t len;
    size_t insert_size;
    kinput_t* input;
    int i;

    // Ensure space in insertbuf
    insert_size = MLE_MAX(6, ctx->bview->buffer->tab_width) * (ctx->pastebuf_len + 1);
    if (ctx->editor->insertbuf_size < insert_size) {
        ctx->editor->insertbuf = realloc(ctx->editor->insertbuf, insert_size);
        ctx->editor->insertbuf_size = insert_size;
    }

    // Fill insertbuf... i=-1: ctx->input, i>=0: ctx->pastebuf
    insertbuf_len = 0;
    insertbuf_cur = ctx->editor->insertbuf;
    for (i = -1; i == -1 || i < ctx->pastebuf_len; i++) {
        input = i == -1 ? &ctx->input : &ctx->pastebuf[i];
        if (input->ch) {
            len = utf8_unicode_to_char(insertbuf_cur, input->ch);
        } else if (input->key == TB_KEY_ENTER || input->key == TB_KEY_CTRL_J || input->key == TB_KEY_CTRL_M) {
            len = sprintf(insertbuf_cur, "\n");
        } else if (input->key >= 0x20 && input->key <= 0x7e) {
            len = sprintf(insertbuf_cur, "%c", input->key);
        } else if (input->key == 0x09) {
            if (ctx->bview->tab_to_space) {
                len = ctx->bview->buffer->tab_width - (ctx->cursor->mark->col % ctx->bview->buffer->tab_width);
                len = sprintf(insertbuf_cur, "%*c", (int)len, ' ');
            } else {
                len = sprintf(insertbuf_cur, "\t");
            }
        } else {
            len = 0; // TODO verbatim input
        }
        insertbuf_cur += len;
        insertbuf_len += len;
    }

    // Write insertbuf to buffer
    if (insertbuf_len < 1) {
        return MLE_ERR;
    }
    ctx->editor->insertbuf[insertbuf_len] = '\0';

    // Insert
    if (insertbuf_len > 1 && ctx->editor->trim_paste && strchr(ctx->editor->insertbuf, '\n') != NULL) {
        // Insert with trim
        char* trimmed = NULL;
        int trimmed_len = 0;
        util_pcre_replace("(?m) +$", ctx->editor->insertbuf, "", &trimmed, &trimmed_len);
        MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_insert_before, trimmed, trimmed_len);
        free(trimmed);
    } else {
        // Insert without trim
        MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_insert_before, ctx->editor->insertbuf, insertbuf_len);
    }

    return MLE_OK;
}

// Insert newline above current line
int cmd_insert_newline_above(cmd_context_t* ctx) {
    MLE_MULTI_CURSOR_CODE(ctx->cursor,
        mark_move_bol(cursor->mark);
        mark_insert_after(cursor->mark, "\n", 1);
    );
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
    uint32_t ch;
    mark_t* mark;
    MLE_MULTI_CURSOR_CODE(ctx->cursor,
        mark_clone(cursor->mark, &mark);
        mark_move_bol(mark);
        mark_get_char_after(mark, &ch);
        if (isspace((int)ch)) {
            mark_move_next_re(mark, "\\S", 2);
        }
        if (mark->col < cursor->mark->col) {
            mark_join(cursor->mark, mark);
        } else {
            mark_move_bol(cursor->mark);
        }
        mark_destroy(mark);
    );
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
    editor_prompt(ctx->editor, "move_to_line: Line num?", NULL, &linestr);
    if (!linestr) return MLE_OK;
    line = strtoll(linestr, NULL, 10);
    free(linestr);
    if (line < 1) line = 1;
    MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_move_to, line - 1, 0);
    bview_center_viewport_y(ctx->bview);
    return MLE_OK;
}

// Move vertically relative to current line
int cmd_move_relative(cmd_context_t* ctx) {
    int delta;
    if (ctx->loop_ctx->numeric_params_len < 1) {
        return MLE_ERR;
    }
    if (strcmp(ctx->static_param, "up") == 0) {
        delta = -1;
    } else if (strcmp(ctx->static_param, "down") == 0) {
        delta = 1;
    } else {
        return MLE_ERR;
    }
    delta *= ctx->loop_ctx->numeric_params[0];
    MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_move_vert, delta);
    return MLE_OK;
}

// Move one word forward
int cmd_move_word_forward(cmd_context_t* ctx) {
    MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_move_next_re, MLE_RE_WORD_FORWARD, sizeof(MLE_RE_WORD_FORWARD)-1);
    return MLE_OK;
}

// Move one word back
int cmd_move_word_back(cmd_context_t* ctx) {
    MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_move_prev_re, MLE_RE_WORD_BACK, sizeof(MLE_RE_WORD_BACK)-1);
    return MLE_OK;
}

// Move to next open bracket
int cmd_move_bracket_forward(cmd_context_t* ctx) {
    MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_move_next_str, "{", 1);
    bview_rectify_viewport(ctx->bview);
    return MLE_OK;
}

// Move to prev open bracket
int cmd_move_bracket_back(cmd_context_t* ctx) {
    MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_move_prev_str, "{", 1);
    bview_rectify_viewport(ctx->bview);
    return MLE_OK;
}

// Delete word back
int cmd_delete_word_before(cmd_context_t* ctx) {
    mark_t* tmark;
    MLE_MULTI_CURSOR_CODE(ctx->cursor,
        mark_clone(cursor->mark, &tmark);
        mark_move_prev_re(tmark, MLE_RE_WORD_BACK, sizeof(MLE_RE_WORD_BACK)-1);
        mark_delete_between_mark(cursor->mark, tmark);
        mark_destroy(tmark);
    );
    return MLE_OK;
}

// Delete word ahead
int cmd_delete_word_after(cmd_context_t* ctx) {
    mark_t* tmark;
    MLE_MULTI_CURSOR_CODE(ctx->cursor,
        mark_clone(cursor->mark, &tmark);
        mark_move_next_re(tmark, MLE_RE_WORD_FORWARD, sizeof(MLE_RE_WORD_FORWARD)-1);
        mark_delete_between_mark(cursor->mark, tmark);
        mark_destroy(tmark);
    );
    return MLE_OK;
}

// Toggle sel bound on cursors
int cmd_toggle_anchor(cmd_context_t* ctx) {
    MLE_MULTI_CURSOR_CODE(ctx->cursor,
        _cmd_toggle_anchor(cursor, 1);
    );
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

// Drop column of cursors in selection
int cmd_drop_cursor_column(cmd_context_t* ctx) {
    bline_t* bline;
    bint_t col;
    mark_t* lo;
    mark_t* hi;
    MLE_MULTI_CURSOR_CODE(ctx->cursor,
        if (!cursor->is_anchored) continue;
        col = cursor->mark->col;
        bview_cursor_get_lo_hi(cursor, &lo, &hi);
        for (bline = lo->bline; bline != hi->bline->next; bline = bline->next) {
            MLBUF_BLINE_ENSURE_CHARS(bline);
            if ((bline == lo->bline && col < lo->col)
                || (bline == hi->bline && col > hi->col)
                || col > bline->char_count
            ) {
                continue;
            }
            if (bline != cursor->mark->bline || col != cursor->mark->col) {
                bview_add_cursor(ctx->bview, bline, col, NULL);
            }
        }
        _cmd_toggle_anchor(cursor, 1);
    );
    return MLE_OK;
}

// Search for a regex
int cmd_search(cmd_context_t* ctx) {
    char* regex;
    int regex_len;
    mark_t* search_mark;
    editor_prompt(ctx->editor, "search: Regex?", NULL, &regex);
    if (!regex) return MLE_OK;
    regex_len = strlen(regex);
    search_mark = buffer_add_mark(ctx->bview->buffer, NULL, 0);
    MLE_MULTI_CURSOR_CODE(ctx->cursor,
        _cmd_search_next(ctx->bview, cursor, search_mark, regex, regex_len);
    );
    mark_destroy(search_mark);
    if (ctx->bview->last_search) free(ctx->bview->last_search);
    ctx->bview->last_search = regex;
    return MLE_OK;
}

// Search for next instance of last search regex
int cmd_search_next(cmd_context_t* ctx) {
    int regex_len;
    mark_t* search_mark;
    if (!ctx->bview->last_search) return MLE_OK;
    regex_len = strlen(ctx->bview->last_search);
    search_mark = buffer_add_mark(ctx->bview->buffer, NULL, 0);
    MLE_MULTI_CURSOR_CODE(ctx->cursor,
        _cmd_search_next(ctx->bview, cursor, search_mark, ctx->bview->last_search, regex_len);
    );
    mark_destroy(search_mark);
    return MLE_OK;
}

// Interactive search and replace
int cmd_replace(cmd_context_t* ctx) {
    char* regex;
    char* replacement;
    int wrapped;
    int all;
    char* yn;
    mark_t* lo_mark;
    mark_t* hi_mark;
    mark_t* orig_mark;
    mark_t* search_mark;
    mark_t* search_mark_end;
    int anchored_before;
    srule_t* highlight;
    bline_t* bline;
    bint_t col;
    bint_t char_count;
    int pcre_rc;
    int pcre_ovector[30];
    char* repl_backref;
    int repl_backref_len;
    int repl_backref_size;
    int num_replacements;

    regex = NULL;
    replacement = NULL;
    wrapped = 0;
    lo_mark = NULL;
    hi_mark = NULL;
    orig_mark = NULL;
    search_mark = NULL;
    search_mark_end = NULL;
    anchored_before = 0;
    all = 0;
    num_replacements = 0;
    mark_set_pcre_capture(&pcre_rc, pcre_ovector, 30);

    do {
        editor_prompt(ctx->editor, "replace: Search regex?", NULL, &regex);
        if (!regex) break;
        editor_prompt(ctx->editor, "replace: Replacement string?", NULL, &replacement);
        if (!replacement) break;
        orig_mark = buffer_add_mark(ctx->bview->buffer, NULL, 0);
        lo_mark = buffer_add_mark(ctx->bview->buffer, NULL, 0);
        hi_mark = buffer_add_mark(ctx->bview->buffer, NULL, 0);
        search_mark = buffer_add_mark(ctx->bview->buffer, NULL, 0);
        search_mark_end = buffer_add_mark(ctx->bview->buffer, NULL, 0);
        mark_join(search_mark, ctx->cursor->mark);
        mark_join(orig_mark, ctx->cursor->mark);
        if (ctx->cursor->is_anchored) {
            anchored_before = mark_is_gt(ctx->cursor->mark, ctx->cursor->anchor);
            mark_join(lo_mark, !anchored_before ? ctx->cursor->mark : ctx->cursor->anchor);
            mark_join(hi_mark, anchored_before ? ctx->cursor->mark : ctx->cursor->anchor);
        } else {
            mark_move_beginning(lo_mark);
            mark_move_end(hi_mark);
        }
        while (1) {
            pcre_rc = 0;
            if (mark_find_next_re(search_mark, regex, strlen(regex), &bline, &col, &char_count) == MLBUF_OK
                && (mark_move_to(search_mark, bline->line_index, col) == MLBUF_OK)
                && (mark_is_gt(search_mark, lo_mark) || mark_is_eq(search_mark, lo_mark))
                && (mark_is_lt(search_mark, hi_mark))
                && (!wrapped || mark_is_lt(search_mark, orig_mark) || mark_is_eq(search_mark, orig_mark))
            ) {
                mark_move_to(search_mark_end, bline->line_index, col + char_count);
                mark_join(ctx->cursor->mark, search_mark);
                yn = NULL;
                if (all) {
                    yn = MLE_PROMPT_YES;
                } else {
                    highlight = srule_new_range(search_mark, search_mark_end, 0, TB_REVERSE);
                    buffer_add_srule(ctx->bview->buffer, highlight);
                    bview_rectify_viewport(ctx->bview);
                    bview_draw(ctx->bview);
                    editor_prompt(ctx->editor, "replace: OK to replace? (y=yes, n=no, a=all, C-c=stop)",
                        &(editor_prompt_params_t) { .kmap = ctx->editor->kmap_prompt_yna }, &yn
                    );
                    buffer_remove_srule(ctx->bview->buffer, highlight);
                    srule_destroy(highlight);
                    bview_draw(ctx->bview);
                }
                if (!yn) {
                    break;
                } else if (0 == strcmp(yn, MLE_PROMPT_YES) || 0 == strcmp(yn, MLE_PROMPT_ALL)) {
                    repl_backref = NULL;
                    repl_backref_len = 0;
                    repl_backref_size = 0;
                    util_replace_with_backrefs(search_mark->bline->data, replacement, pcre_rc, pcre_ovector, 30, &repl_backref, &repl_backref_len, &repl_backref_size);
                    mark_delete_between_mark(search_mark, search_mark_end);
                    if (repl_backref) {
                        mark_insert_before(search_mark, repl_backref, repl_backref_len);
                        free(repl_backref);
                    }
                    num_replacements += 1;
                    if (0 == strcmp(yn, MLE_PROMPT_ALL)) all = 1;
                } else {
                    mark_move_by(search_mark, 1);
                }
            } else if (!wrapped) {
                mark_join(search_mark, lo_mark);
                mark_move_by(search_mark, -1);
                wrapped = 1;
            } else {
                break;
            }
        }
    } while(0);

    if (ctx->cursor->is_anchored && lo_mark && hi_mark) {
        mark_join(ctx->cursor->mark, anchored_before ? hi_mark : lo_mark);
        mark_join(ctx->cursor->anchor, anchored_before ? lo_mark : hi_mark);
    }

    mark_set_pcre_capture(NULL, NULL, 0);
    if (regex) free(regex);
    if (replacement) free(replacement);
    if (lo_mark) mark_destroy(lo_mark);
    if (hi_mark) mark_destroy(hi_mark);
    if (orig_mark) mark_destroy(orig_mark);
    if (search_mark) mark_destroy(search_mark);
    if (search_mark_end) mark_destroy(search_mark_end);

    MLE_SET_INFO(ctx->editor, "replace: Replaced %d instance(s)", num_replacements);

    bview_rectify_viewport(ctx->bview);
    bview_draw(ctx->bview);

    return MLE_OK;
}

// Redraw screen
int cmd_redraw(cmd_context_t* ctx) {
    int w;
    int h;
    int x;
    int y;
    bview_center_viewport_y(ctx->bview);
    w = tb_width();
    h = tb_height();
    for (x = 0; x < w; x++) {
        for (y = 0; y < h; y++) {
            tb_change_cell(x, y, 160, 0, 0);
        }
    }
    tb_present();
    return MLE_OK;
}

// Find next occurence of word under cursor
int cmd_find_word(cmd_context_t* ctx) {
    char* re;
    char* word;
    bint_t re_len;
    bint_t word_len;
    MLE_MULTI_CURSOR_CODE(ctx->cursor,
        if (_cmd_select_by(cursor, "word") == MLE_OK) {
            mark_get_between_mark(cursor->mark, cursor->anchor, &word, &word_len);
            re_len = asprintf(&re, "\\b%s\\b", word);
            free(word);
            _cmd_toggle_anchor(cursor, 0);
            if (mark_move_next_re(cursor->mark, re, re_len) == MLBUF_ERR) {
                mark_move_beginning(cursor->mark);
                mark_move_next_re(cursor->mark, re, re_len);
            }
            free(re);
        }
    );
    bview_rectify_viewport(ctx->bview);
    return MLE_OK;
}

// Incremental search
int cmd_isearch(cmd_context_t* ctx) {
    editor_prompt(ctx->editor, "isearch: Regex?", &(editor_prompt_params_t) {
        .kmap = ctx->editor->kmap_prompt_isearch,
        .prompt_cb = _cmd_isearch_prompt_cb
    }, NULL);
    if (ctx->bview->isearch_rule) {
        buffer_remove_srule(ctx->bview->buffer, ctx->bview->isearch_rule);
        srule_destroy(ctx->bview->isearch_rule);
        ctx->bview->isearch_rule = NULL;
    }
    return MLE_OK;
}

// Cut text
int cmd_cut(cmd_context_t* ctx) {
    int append;
    append = ctx->loop_ctx->last_cmd && ctx->loop_ctx->last_cmd->func == cmd_cut ? 1 : 0;
    MLE_MULTI_CURSOR_CODE(ctx->cursor,
        _cmd_cut_copy(cursor, 1, 1, append);
    );
    return MLE_OK;
}

// Copy text
int cmd_copy(cmd_context_t* ctx) {
    MLE_MULTI_CURSOR_CODE(ctx->cursor,
        _cmd_cut_copy(cursor, 0, 1, 0);
    );
    return MLE_OK;
}

// Paste text
int cmd_uncut(cmd_context_t* ctx) {
    MLE_MULTI_CURSOR_CODE(ctx->cursor,
        if (!cursor->cut_buffer) continue;
        mark_insert_before(cursor->mark, cursor->cut_buffer, strlen(cursor->cut_buffer));
    );
    return MLE_OK;
}

// Copy in between chars
int cmd_copy_by(cmd_context_t* ctx) {
    MLE_MULTI_CURSOR_CODE(ctx->cursor,
        if (_cmd_select_by(cursor, ctx->static_param) == MLE_OK) {
            _cmd_cut_copy(cursor, 0, 0, 0);
        }
    );
    return MLE_OK;
}

// Cut in between chars
int cmd_cut_by(cmd_context_t* ctx) {
    MLE_MULTI_CURSOR_CODE(ctx->cursor,
        if (_cmd_select_by(cursor, ctx->static_param) == MLE_OK) {
            _cmd_cut_copy(cursor, 1, 0, 0);
        }
    );
    return MLE_OK;
}

// Go to next bview
int cmd_next(cmd_context_t* ctx) {
    editor_set_active(ctx->editor, ctx->bview->all_prev);
    return MLE_OK;
}

// Go to prev bview
int cmd_prev(cmd_context_t* ctx) {
    editor_set_active(ctx->editor, ctx->bview->all_next);
    return MLE_OK;
}

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

// Fuzzy path search via fzf
int cmd_fsearch(cmd_context_t* ctx) {
    async_proc_t* aproc;
    char* path;
    int replace_view;
    replace_view = ctx->static_param && strcmp(ctx->static_param, "replace") == 0 ? 1 : 0;
    if (replace_view && _cmd_pre_close(ctx->editor, ctx->bview) == MLE_ERR) return MLE_OK;
    aproc = async_proc_new(ctx->bview, 1, 0, _cmd_aproc_passthru_cb, "fzf -f '' 2>/dev/null");
    editor_prompt_menu(ctx->editor, "fsearch: Fuzzy path?", NULL, 0, _cmd_fsearch_prompt_cb, aproc, &path);
    if (replace_view) {
        bview_open(ctx->bview, path, path ? strlen(path) : 0);
        bview_resize(ctx->bview, ctx->bview->x, ctx->bview->y, ctx->bview->w, ctx->bview->h);
    } else if (path) {
        editor_open_bview(ctx->editor, NULL, MLE_BVIEW_TYPE_EDIT, path, strlen(path), 1, 0, &ctx->editor->rect_edit, NULL, NULL);
    }
    if (path) free(path);
    return MLE_OK;
}

// Grep for pattern in cwd
int cmd_grep(cmd_context_t* ctx) {
    async_proc_t* aproc;
    char* path;
    char* path_arg;
    char* cmd;
    char* grep_fmt;
    editor_prompt(ctx->editor, "grep: Pattern?", NULL, &path);
    if (!path) return MLE_OK;
    if (ctx->static_param) {
        grep_fmt = ctx->static_param;
    } else {
        grep_fmt = "grep --color=never -P -i -I -n -r %s . 2>/dev/null";
    }
    path_arg = util_escape_shell_arg(path, strlen(path));
    free(path);
    asprintf(&cmd, grep_fmt, path_arg);
    free(path_arg);
    if (!cmd) {
        MLE_RETURN_ERR(ctx->editor, "Failed to format grep cmd: %s", grep_fmt);
    }
    aproc = async_proc_new(ctx->bview, 1, 0, _cmd_aproc_passthru_cb, cmd);
    free(cmd);
    editor_menu(ctx->editor, _cmd_grep_cb, NULL, 0, aproc, NULL);
    return MLE_OK;
}

// Browse directory via tree
int cmd_browse(cmd_context_t* ctx) {
    bview_t* menu;
    async_proc_t* aproc;
    char* cmd;
    asprintf(&cmd, "tree --charset C -n -f -L 2 %s 2>/dev/null", ctx->static_param ? ctx->static_param : "");
    aproc = async_proc_new(ctx->bview, 1, 0, _cmd_aproc_passthru_cb, cmd);
    free(cmd);
    editor_menu(ctx->editor, _cmd_browse_cb, "..\n", 3, aproc, &menu);
    mark_move_beginning(menu->active_cursor->mark);
    return MLE_OK;
}

// Save-as file
int cmd_save_as(cmd_context_t* ctx) {
    _cmd_save(ctx->editor, ctx->bview, 1);
    return MLE_OK;
}

// Save file
int cmd_save(cmd_context_t* ctx) {
    _cmd_save(ctx->editor, ctx->bview, 0);
    return MLE_OK;
}

// Open file in a new bview
int cmd_open_file(cmd_context_t* ctx) {
    char* path;
    editor_prompt(ctx->editor, "new_open: File?", NULL, &path);
    if (!path) return MLE_OK;
    editor_open_bview(ctx->editor, NULL, MLE_BVIEW_TYPE_EDIT, path, strlen(path), 1, 0, &ctx->editor->rect_edit, NULL, NULL);
    free(path);
    return MLE_OK;
}

// Open empty buffer in a new bview
int cmd_open_new(cmd_context_t* ctx) {
    editor_open_bview(ctx->editor, NULL, MLE_BVIEW_TYPE_EDIT, NULL, 0, 1, 0, &ctx->editor->rect_edit, NULL, NULL);
    return MLE_OK;
}

// Open file into current bview
int cmd_open_replace_file(cmd_context_t* ctx) {
    char* path;
    if (_cmd_pre_close(ctx->editor, ctx->bview) == MLE_ERR) return MLE_OK;
    path = NULL;
    editor_prompt(ctx->editor, "replace_open: Path?", NULL, &path);
    if (!path) return MLE_OK;
    bview_open(ctx->bview, path, strlen(path));
    bview_resize(ctx->bview, ctx->bview->x, ctx->bview->y, ctx->bview->w, ctx->bview->h);
    free(path);
    return MLE_OK;
}

// Open empty buffer into current bview
int cmd_open_replace_new(cmd_context_t* ctx) {
    if (_cmd_pre_close(ctx->editor, ctx->bview) == MLE_ERR) return MLE_OK;
    bview_open(ctx->bview, NULL, 0);
    bview_resize(ctx->bview, ctx->bview->x, ctx->bview->y, ctx->bview->w, ctx->bview->h);
    return MLE_OK;
}

// Close bview
int cmd_close(cmd_context_t* ctx) {
    int num_open;
    int num_closed;
    if (_cmd_pre_close(ctx->editor, ctx->bview) == MLE_ERR) return MLE_OK;
    num_open = editor_bview_edit_count(ctx->editor);
    editor_close_bview(ctx->editor, ctx->bview, &num_closed);
    ctx->loop_ctx->should_exit = num_closed == num_open ? 1 : 0;
    return MLE_OK;
}

// Quit editor
int cmd_quit(cmd_context_t* ctx) {
    bview_t* bview;
    bview_t* tmp;
    if (ctx->editor->loop_depth > 1) return MLE_OK;
    DL_FOREACH_SAFE2(ctx->editor->top_bviews, bview, tmp, top_next) {
        if (!MLE_BVIEW_IS_EDIT(bview)) {
            continue;
        } else if (_cmd_quit_inner(ctx->editor, bview) == MLE_ERR) {
            return MLE_OK;
        }
    }
    ctx->loop_ctx->should_exit = 1;
    return MLE_OK;
}

// Apply a macro with single-char name
int cmd_apply_macro_by(cmd_context_t* ctx) {
    kmacro_t* macro;
    uint32_t ch;
    char name[6] = { 0 };
    ch = MLE_PARAM_WILDCARD(ctx, 0);
    if (!ch) return MLE_OK;
    utf8_unicode_to_char(name, ch);
    HASH_FIND_STR(ctx->editor->macro_map, name, macro);
    if (!macro) MLE_RETURN_ERR(ctx->editor, "Macro not found with name '%s'", name);
    ctx->editor->macro_apply = macro;
    ctx->editor->macro_apply_input_index = 0;
    return MLE_OK;
}


// Apply a macro
int cmd_apply_macro(cmd_context_t* ctx) {
    char* name;
    kmacro_t* macro;
    if (ctx->editor->macro_apply) MLE_RETURN_ERR(ctx->editor, "Cannot nest macros%s", "");
    editor_prompt(ctx->editor, "apply_macro: Name?", NULL, &name);
    if (!name) return MLE_OK;
    HASH_FIND_STR(ctx->editor->macro_map, name, macro);
    free(name);
    if (!macro) MLE_RETURN_ERR(ctx->editor, "Macro not found%s", "");
    ctx->editor->macro_apply = macro;
    ctx->editor->macro_apply_input_index = 0;
    return MLE_OK;
}

// No-op
int cmd_noop(cmd_context_t* ctx) {
    return MLE_OK;
}

// Move forward til a certain char
int cmd_move_until_forward(cmd_context_t* ctx) {
    uint32_t ch;
    char str[6] = { 0 };
    ch = MLE_PARAM_WILDCARD(ctx, 0);
    if (!ch) return MLE_OK;
    utf8_unicode_to_char(str, ch);
    MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_move_next_str, str, strlen(str));
    bview_rectify_viewport(ctx->bview);
    return MLE_OK;
}

// Move back til a certain char
int cmd_move_until_back(cmd_context_t* ctx) {
    uint32_t ch;
    char str[6] = { 0 };
    ch = MLE_PARAM_WILDCARD(ctx, 0);
    if (!ch) return MLE_OK;
    utf8_unicode_to_char(str, ch);
    MLE_MULTI_CURSOR_MARK_FN(ctx->cursor, mark_move_prev_str, str, strlen(str));
    bview_rectify_viewport(ctx->bview);
    return MLE_OK;
}

// Undo
int cmd_undo(cmd_context_t* ctx) {
    buffer_undo(ctx->bview->buffer);
    return MLE_OK;
}

// Redo
int cmd_redo(cmd_context_t* ctx) {
    buffer_redo(ctx->bview->buffer);
    return MLE_OK;
}

// Indent line(s)
int cmd_indent(cmd_context_t* ctx) {
    return _cmd_indent(ctx, 0);
}

// Outdent line(s)
int cmd_outdent(cmd_context_t* ctx) {
    return _cmd_indent(ctx, 1);
}

// Set option
int cmd_set_opt(cmd_context_t* ctx) {
    char* prompt;
    char* val;
    int vali;
    if (!ctx->static_param) return MLE_ERR;
    asprintf(&prompt, "set_opt: %s?", ctx->static_param);
    editor_prompt(ctx->editor, prompt, NULL, &val);
    free(prompt);
    if (!val) return MLE_OK;
    vali = atoi(val);
    if (strcmp(ctx->static_param, "tab_to_space") == 0) {
        ctx->bview->tab_to_space = vali ? 1 : 0;
    } else if (strcmp(ctx->static_param, "tab_width") == 0) {
        ctx->bview->tab_width = MLE_MAX(vali, 1);
        buffer_set_tab_width(ctx->bview->buffer, ctx->bview->tab_width);
    } else if (strcmp(ctx->static_param, "syntax") == 0) {
        bview_set_syntax(ctx->bview, val);
        buffer_apply_styles(ctx->bview->buffer, ctx->bview->buffer->first_line, ctx->bview->buffer->line_count);
    } else if (strcmp(ctx->static_param, "soft_wrap") == 0) {
        ctx->editor->soft_wrap = vali ? 1 : 0;
    }
    return MLE_OK;
}

// Shell
int cmd_shell(cmd_context_t* ctx) {
    char* cmd;
    char* input;
    bint_t input_len;
    char* output;
    size_t output_len;

    // Get shell cmd
    if (ctx->static_param) {
        cmd = strdup(ctx->static_param);
    } else {
        editor_prompt(ctx->editor, "shell: Cmd?", NULL, &cmd);
        if (!cmd) return MLE_OK;
    }

    // Loop for each cursor
    MLE_MULTI_CURSOR_CODE(ctx->cursor,
        // Get data to send to stdin
        if (ctx->cursor->is_anchored) {
            mark_get_between_mark(cursor->mark, cursor->anchor, &input, &input_len);
            // Add a newline
            input = realloc(input, input_len + 2);
            input[input_len] = '\n';
            input[input_len+1] = '\0';
            input_len += 1;
        } else {
            input = NULL;
            input_len = 0;
        }

        // Run cmd
        output = NULL;
        output_len = 0;
        if (util_shell_exec(ctx->editor, cmd, 1, input, input_len, NULL, &output, &output_len) == MLE_OK && output_len > 0) {
            // Write output to buffer
            if (cursor->is_anchored) {
                mark_delete_between_mark(cursor->mark, cursor->anchor);
            }
            mark_insert_before(cursor->mark, output, output_len);
        }

        // Free input and output
        if (input) free(input);
        if (output) free(output);
    ); // Loop for next cursor

    free(cmd);
    return MLE_OK;
}

// Indent or outdent line(s)
static int _cmd_indent(cmd_context_t* ctx, int outdent) {
    bline_t* start;
    bline_t* end;
    bline_t* cur;
    int use_tabs;
    use_tabs = ctx->bview->tab_to_space ? 0 : 1;
    MLE_MULTI_CURSOR_CODE(ctx->cursor,
        start = ctx->cursor->mark->bline;
        if (ctx->cursor->is_anchored) {
            end = ctx->cursor->anchor->bline;
            if (start->line_index > end->line_index) {
                cur = end;
                end = start;
                start = cur;
            }
        } else {
            end = start;
        }
        ctx->buffer->is_style_disabled++;
        for (cur = start; cur != end->next; cur = cur->next) {
            _cmd_indent_line(cur, use_tabs, outdent);
        }
        ctx->buffer->is_style_disabled--;
        buffer_apply_styles(ctx->buffer, start, 0);
    );
    return MLE_OK;
}

// Push kmap
int cmd_push_kmap(cmd_context_t* ctx) {
    kmap_t* kmap;
    char* kmap_name;
    int rv;

    // Get kmap name
    kmap_name = NULL;
    if (ctx->static_param) {
        kmap_name = strdup(ctx->static_param);
    } else {
        editor_prompt(ctx->editor, "push_kmap: Kmap name?", NULL, &kmap_name);
        if (!kmap_name) return MLE_OK;
    }

    // Find kmap by name
    kmap = NULL;
    HASH_FIND_STR(ctx->editor->kmap_map, kmap_name, kmap);

    if (!kmap) {
        // Not found
        MLE_SET_ERR(ctx->editor, "push_kmap: No kmap defined '%s'", kmap_name);
        rv = MLE_ERR;
    } else {
        // Found, push
        bview_push_kmap(ctx->bview, kmap);
        rv = MLE_OK;
    }

    free(kmap_name);
    return rv;
}

// Pop kmap
int cmd_pop_kmap(cmd_context_t* ctx) {
    bview_pop_kmap(ctx->bview, NULL);
    return MLE_OK;
}

// Show help
int cmd_show_help(cmd_context_t* ctx) {
    char* h; // help
    int l; // len
    int s; // size
    kmap_t* kmap;
    kmap_t* kmap_tmp;
    kmap_node_t* kmap_node;
    int kmap_node_depth;
    int i;
    bview_t* bview;
    char buf[1024];

    h = NULL;
    l = 0;
    s = 0;

    util_str_append(
        "# mle command help\n\n"
        "    notes\n"
        "        C- means Ctrl\n"
        "        M- means Alt\n"
        "        cmd_x=(default) means unmatched input is handled by cmd_x\n"
        "        (allow_fallthru)=yes means unmatched input is handled by the parent kmap\n\n",
        NULL,
        &h, &l, &s
    );

    // Build current kmap stack
    util_str_append("    mode stack\n", NULL, &h, &l, &s);
    kmap_node_depth = 0;
    DL_FOREACH(ctx->bview->kmap_stack, kmap_node) {
        util_str_append("        ", NULL, &h, &l, &s);
        for (i = 0; i < kmap_node_depth; i++) {
            util_str_append("    ", NULL, &h, &l, &s);
        }
        if (i > 0) {
            util_str_append("\\_ ", NULL, &h, &l, &s);
        }
        util_str_append(kmap_node->kmap->name, NULL, &h, &l, &s);
        if (kmap_node == ctx->bview->kmap_tail) {
            util_str_append(" (current)", NULL, &h, &l, &s);
        }
        util_str_append("\n", NULL, &h, &l, &s);
        kmap_node_depth += 1;
    }
    util_str_append("\n", NULL, &h, &l, &s);

    // Build kmap bindings
    HASH_ITER(hh, ctx->editor->kmap_map, kmap, kmap_tmp) {
        util_str_append("    ", NULL, &h, &l, &s);
        util_str_append(kmap->name, NULL, &h, &l, &s);
        util_str_append(" mode\n", NULL, &h, &l, &s);
        snprintf(buf, sizeof(buf), "        %-40s %-16s\n", "(allow_fallthru)", kmap->allow_fallthru ? "yes" : "no");
        util_str_append(buf, NULL, &h, &l, &s);
        if (kmap->default_cmd_name) {
            snprintf(buf, sizeof(buf), "        %-40s %-16s\n", kmap->default_cmd_name, "(default)");
            util_str_append(buf, NULL, &h, &l, &s);
        }
        _cmd_help_inner(buf, kmap->bindings->children, &h, &l, &s);
        util_str_append("\n", NULL, &h, &l, &s);
    }

    // Show help in new bview
    editor_open_bview(ctx->editor, NULL, MLE_BVIEW_TYPE_EDIT, NULL, 0, 1, 0, &ctx->editor->rect_edit, NULL, &bview);
    buffer_insert(bview->buffer, 0, h, (bint_t)l, NULL);
    bview->buffer->is_unsaved = 0;
    mark_move_beginning(bview->active_cursor->mark);
    bview_zero_viewport_y(bview);

    free(h);
    return MLE_OK;
}

// Recursively descend into kbinding trie to build help string
static void _cmd_help_inner(char* buf, kbinding_t* trie, char** h, int* l, int* s) {
    kbinding_t* binding;
    kbinding_t* binding_tmp;
    HASH_ITER(hh, trie, binding, binding_tmp) {
        if (binding->children) {
            _cmd_help_inner(buf, binding->children, h, l, s);
        } else if (binding->is_leaf) {
            snprintf(buf, 1024,
                "        %-40s %-16s %s\n",
                binding->cmd_name,
                binding->key_patt,
                binding->static_param ? binding->static_param : ""
            );
            util_str_append(buf, NULL, h, l, s);
        }
    }
}

// Indent/outdent a line, optionally using tabs
static int _cmd_indent_line(bline_t* bline, int use_tabs, int outdent) {
    char tab_char;
    int num_chars;
    int num_to_del;
    int i;
    bint_t ig;
    tab_char = use_tabs ? '\t' : ' ';
    num_chars = use_tabs ? 1 : bline->buffer->tab_width;
    MLBUF_BLINE_ENSURE_CHARS(bline);
    if (outdent) {
        num_to_del = 0;
        for (i = 0; i < num_chars; i++) {
            if (bline->char_count > i && bline->chars[i].ch == tab_char) {
                num_to_del += 1;
            } else {
                break;
            }
        }
        if (num_to_del > 0) bline_delete(bline, 0, num_to_del);
    } else {
        if (bline->char_count > 0) {
            for (i = 0; i < num_chars; i++) {
                bline_insert(bline, 0, &tab_char, 1, &ig);
            }
        }
    }
    return MLE_OK;
}

// Place marks for cmd_(cut|copy)_by
static int _cmd_select_by(cursor_t* cursor, char* strat) {
    if (cursor->is_anchored) {
        return MLE_ERR;
    }
    if (strcmp(strat, "bracket") == 0) {
        return _cmd_select_by_bracket(cursor);
    } else if (strcmp(strat, "word") == 0) {
        return _cmd_select_by_word(cursor);
    } else if (strcmp(strat, "word_back") == 0) {
        return _cmd_select_by_word_back(cursor);
    } else if (strcmp(strat, "word_forward") == 0) {
        return _cmd_select_by_word_forward(cursor);
    } else if (strcmp(strat, "eol") == 0 && !mark_is_at_eol(cursor->mark)) {
        _cmd_toggle_anchor(cursor, 0);
        mark_move_eol(cursor->anchor);
    } else if (strcmp(strat, "bol") == 0 && !mark_is_at_bol(cursor->mark)) {
        _cmd_toggle_anchor(cursor, 0);
        mark_move_bol(cursor->anchor);
    } else if (strcmp(strat, "string") == 0) {
        return _cmd_select_by_string(cursor);
    } else {
        MLE_RETURN_ERR(cursor->bview->editor, "Unrecognized _cmd_select_by strat '%s'", strat);
    }
    return MLE_OK;
}

// Select by bracket
static int _cmd_select_by_bracket(cursor_t* cursor) {
    mark_t* orig;
    mark_clone(cursor->mark, &orig);
    if (mark_move_bracket_top(cursor->mark, MLE_BRACKET_PAIR_MAX_SEARCH) != MLBUF_OK) {
        mark_destroy(orig);
        return MLE_ERR;
    }
    _cmd_toggle_anchor(cursor, 0);
    if (mark_move_bracket_pair(cursor->anchor, MLE_BRACKET_PAIR_MAX_SEARCH) != MLBUF_OK) {
        _cmd_toggle_anchor(cursor, 0);
        mark_join(cursor->mark, orig);
        mark_destroy(orig);
        return MLE_ERR;
    }
    mark_move_by(cursor->mark, 1);
    mark_destroy(orig);
    return MLE_OK;
}

// Select by word-back
static int _cmd_select_by_word_back(cursor_t* cursor) {
    if (mark_is_at_word_bound(cursor->mark, -1)) return MLE_ERR;
    _cmd_toggle_anchor(cursor, 0);
    mark_move_prev_re(cursor->mark, MLE_RE_WORD_BACK, sizeof(MLE_RE_WORD_BACK)-1);
    return MLE_OK;
}

// Select by word-forward
static int _cmd_select_by_word_forward(cursor_t* cursor) {
    if (mark_is_at_word_bound(cursor->mark, 1)) return MLE_ERR;
    _cmd_toggle_anchor(cursor, 0);
    mark_move_next_re(cursor->mark, MLE_RE_WORD_FORWARD, sizeof(MLE_RE_WORD_FORWARD)-1);
    return MLE_OK;
}

// Select by string
static int _cmd_select_by_string(cursor_t* cursor) {
    mark_t* orig;
    uint32_t qchar;
    char* qre;
    mark_clone(cursor->mark, &orig);
    if (mark_move_prev_re(cursor->mark, "(?<!\\\\)[`'\"]", strlen("(?<!\\\\)[`'\"]")) != MLBUF_OK) {
        mark_destroy(orig);
        return MLE_ERR;
    }
    _cmd_toggle_anchor(cursor, 0);
    mark_get_char_after(cursor->mark, &qchar);
    mark_move_by(cursor->mark, 1);
    if (qchar == '"') {
        qre = "(?<!\\\\)\"";
    } else if (qchar == '\'') {
        qre = "(?<!\\\\)'";
    } else {
        qre = "(?<!\\\\)`";
    }
    if (mark_move_next_re(cursor->anchor, qre, strlen(qre)) != MLBUF_OK) {
        _cmd_toggle_anchor(cursor, 0);
        mark_join(cursor->mark, orig);
        mark_destroy(orig);
        return MLE_ERR;
    }
    mark_destroy(orig);
    return MLE_OK;
}

// Select by word
static int _cmd_select_by_word(cursor_t* cursor) {
    uint32_t after;
    if (mark_is_at_eol(cursor->mark)) return MLE_ERR;
    mark_get_char_after(cursor->mark, &after);
    if (!isalnum((char)after) && (char)after != '_') return MLE_ERR;
    if (!mark_is_at_word_bound(cursor->mark, -1)) {
        mark_move_prev_re(cursor->mark, MLE_RE_WORD_BACK, sizeof(MLE_RE_WORD_BACK)-1);
    }
    _cmd_toggle_anchor(cursor, 0);
    mark_move_next_re(cursor->mark, MLE_RE_WORD_FORWARD, sizeof(MLE_RE_WORD_FORWARD)-1);
    return MLE_OK;
}

// Recursively close bviews, prompting to save unsaved changes. Return MLE_OK if
// it's OK to continue closing, or MLE_ERR if the action was cancelled.
static int _cmd_quit_inner(editor_t* editor, bview_t* bview) {
    if (bview->split_child && _cmd_quit_inner(editor, bview->split_child) == MLE_ERR) {
        return MLE_ERR;
    } else if (_cmd_pre_close(editor, bview) == MLE_ERR) {
        return MLE_ERR;
    }
    editor_close_bview(editor, bview, NULL);
    return MLE_OK;
}

// Prompt to save unsaved changes on close. Return MLE_OK if it's OK to continue
// closing the bview, or MLE_ERR if the action was cancelled.
static int _cmd_pre_close(editor_t* editor, bview_t* bview) {
    char* yn;

    if (!MLE_BVIEW_IS_EDIT(bview)) {
        MLE_RETURN_ERR(editor, "Cannot close non-edit bview %p", bview);
    } else if (editor->loop_depth > 1) {
        MLE_RETURN_ERR(editor, "Cannot close bview %p when loop_depth > 1", bview);
    } else if (!bview->buffer->is_unsaved || MLE_BVIEW_IS_MENU(bview)
        || editor_count_bviews_by_buffer(editor, bview->buffer) > 1
    ) {
        return MLE_OK;
    }

    editor_set_active(editor, bview);

    yn = NULL;
    editor_prompt(editor, "close: Save modified? (y=yes, n=no, C-c=cancel)",
        &(editor_prompt_params_t) { .kmap = editor->kmap_prompt_yn }, &yn
    );
    if (!yn) {
        return MLE_ERR;
    } else if (0 == strcmp(yn, MLE_PROMPT_NO)) {
        return MLE_OK;
    }
    return _cmd_save(editor, bview, 1);
}

// Prompt to save changes. Return MLE_OK if file was saved or MLE_ERR if the action
// was cancelled.
static int _cmd_save(editor_t* editor, bview_t* bview, int save_as) {
    int rc;
    char* path;
    char* yn;
    int fname_changed;
    struct stat st;
    bint_t nbytes;

    fname_changed = 0;
    do {
        if (!bview->buffer->path || save_as) {
            // Prompt for name
            editor_prompt(editor, "save: Save as? (C-c=cancel)", &(editor_prompt_params_t) {
                .data = bview->buffer->path ? bview->buffer->path : "",
                .data_len = bview->buffer->path ? strlen(bview->buffer->path) : 0
            }, &path);
            if (!path) return MLE_ERR;
        } else {
            // Re-use existing name
            path = strdup(bview->buffer->path);
        }

        // Remember if fname is changing for refreshing syntax later
        fname_changed = !bview->buffer->path || strcmp(bview->buffer->path, path) != 0 ? 1 : 0;

        // Check clobber warning
        if (stat(path, &st) == 0
            && st.st_dev == bview->buffer->st.st_dev
            && st.st_ino == bview->buffer->st.st_ino
            && st.st_mtime > bview->buffer->st.st_mtime
        ) {
            // File was modified after it was opened/last saved
            editor_prompt(editor, "save: Clobber detected! Continue? (y=yes, n=no)",
                &(editor_prompt_params_t) { .kmap = editor->kmap_prompt_yn }, &yn
            );
            if (!yn || 0 == strcmp(yn, MLE_PROMPT_NO)) {
                free(path);
                return MLE_OK;
            }
        }

        // Save, check error
        rc = buffer_save_as(bview->buffer, path, &nbytes);
        free(path);
        if (rc == MLBUF_ERR) {
            MLE_SET_ERR(editor, "save: %s", errno ? strerror(errno) : "failed");
        } else {
            MLE_SET_INFO(editor, "save: Wrote %ld bytes", nbytes);
        }

    } while (rc == MLBUF_ERR && (!bview->buffer->path || save_as));

    // Refresh syntax if fname changed
    if (fname_changed) {
        bview_set_syntax(bview, NULL);
    }
    return rc == MLBUF_OK ? MLE_OK : MLE_ERR;
}

// Cut or copy text
static void _cmd_cut_copy(cursor_t* cursor, int is_cut, int use_srules, int append) {
    char* cutbuf;
    bint_t cutbuf_len;
    if (!append && cursor->cut_buffer) {
        free(cursor->cut_buffer);
        cursor->cut_buffer = NULL;
    }
    if (!cursor->is_anchored) {
        use_srules = 0;
        _cmd_toggle_anchor(cursor, use_srules);
        mark_move_bol(cursor->mark);
        mark_move_eol(cursor->anchor);
        mark_move_by(cursor->anchor, 1);
    }
    mark_get_between_mark(cursor->mark, cursor->anchor, &cutbuf, &cutbuf_len);
    if (append && cursor->cut_buffer) {
        bint_t cur_len = strlen(cursor->cut_buffer);
        cursor->cut_buffer = realloc(cursor->cut_buffer, cur_len + cutbuf_len + 1);
        strncat(cursor->cut_buffer, cutbuf, cutbuf_len);
        free(cutbuf);
    } else {
        cursor->cut_buffer = cutbuf;
    }
    if (is_cut) {
        mark_delete_between_mark(cursor->mark, cursor->anchor);
    }
    _cmd_toggle_anchor(cursor, use_srules);
}

// Anchor/unanchor cursor selection bound
static void _cmd_toggle_anchor(cursor_t* cursor, int use_srules) {
    if (!cursor->is_anchored) {
        mark_clone(cursor->mark, &(cursor->anchor));
        if (use_srules) {
            cursor->sel_rule = srule_new_range(cursor->mark, cursor->anchor, 0, TB_REVERSE);
            buffer_add_srule(cursor->bview->buffer, cursor->sel_rule);
        }
        cursor->is_anchored = 1;
    } else {
        if (use_srules) {
            buffer_remove_srule(cursor->bview->buffer, cursor->sel_rule);
            srule_destroy(cursor->sel_rule);
            cursor->sel_rule = NULL;
        }
        mark_destroy(cursor->anchor);
        cursor->is_anchored = 0;
    }
}

// Move cursor to next occurrence of term, wrap if necessary. Return MLE_OK if
// there was a match, or MLE_ERR if no match.
static int _cmd_search_next(bview_t* bview, cursor_t* cursor, mark_t* search_mark, char* regex, int regex_len) {
    int rc;
    rc = MLE_ERR;

    // Move search_mark to cursor
    mark_join(search_mark, cursor->mark);
    // Look for match ahead of us
    if (mark_move_next_re(search_mark, regex, regex_len) == MLBUF_OK) {
        // Match! Move there
        mark_join(cursor->mark, search_mark);
        rc = MLE_OK;
    } else {
        // No match, try from beginning
        mark_move_beginning(search_mark);
        if (mark_move_next_re(search_mark, regex, regex_len) == MLBUF_OK) {
            // Match! Move there
            mark_join(cursor->mark, search_mark);
            rc = MLE_OK;
        }
    }

    // Rectify viewport if needed
    if (rc == MLE_OK) bview_rectify_viewport(bview);

    return rc;
}

// Aproc callback that writes buf to bview buffer
static void _cmd_aproc_passthru_cb(async_proc_t* aproc, char* buf, size_t buf_len, int is_error, int is_eof, int is_timeout) {
    mark_t* active_mark;
    mark_t* ins_mark;
    int is_cursor_at_zero;
    if (!buf || buf_len < 1) return;

    // Remeber if cursor is at 0
    active_mark = aproc->invoker->active_cursor->mark;
    is_cursor_at_zero = active_mark->bline->line_index == 0 && active_mark->col == 0 ? 1 : 0;

    // Append data at end of menu buffer
    ins_mark = buffer_add_mark(aproc->invoker->buffer, NULL, 0);
    mark_move_end(ins_mark);
    mark_insert_before(ins_mark, buf, buf_len);
    mark_destroy(ins_mark);
    bview_rectify_viewport(aproc->invoker);

    if (is_cursor_at_zero) mark_move_beginning(active_mark);
}

// Incremental search prompt callback
static void _cmd_isearch_prompt_cb(bview_t* bview_prompt, baction_t* action, void* udata) {
    bview_t* bview;
    char* regex;
    int regex_len;

    bview = bview_prompt->editor->active_edit;

    if (bview->isearch_rule) {
        buffer_remove_srule(bview->buffer, bview->isearch_rule);
        srule_destroy(bview->isearch_rule);
        bview->isearch_rule = NULL;
    }

    regex = bview_prompt->buffer->first_line->data;
    regex_len = bview_prompt->buffer->first_line->data_len;
    if (regex_len < 1) return;

    bview->isearch_rule = srule_new_single(regex, regex_len, 1, 0, TB_YELLOW);
    if (!bview->isearch_rule) return;

    buffer_add_srule(bview->buffer, bview->isearch_rule);
    mark_move_by(bview->active_cursor->mark, -1);
    if (mark_move_next_cre(bview->active_cursor->mark, bview->isearch_rule->cre) != MLBUF_OK) {
        mark_move_by(bview->active_cursor->mark, 1);
    }

    bview_center_viewport_y(bview);
}

// Fuzzy path search prompt callback
static void _cmd_fsearch_prompt_cb(bview_t* bview_prompt, baction_t* action, void* udata) {
    async_proc_t* aproc;
    bview_t* menu;
    char* shell_cmd;
    char* shell_arg;

    // Get menu and aproc
    menu = bview_prompt->editor->active_edit;
    aproc = menu->async_proc;
    if (aproc && aproc->invoker == menu) {
        // Mark current aproc is_done
        aproc->is_done = 1;
    }

    // Clear menu
    buffer_set(menu->buffer, "", 0);

    // Make new aproc
    shell_arg = util_escape_shell_arg(bview_prompt->buffer->first_line->data, bview_prompt->buffer->first_line->data_len);
    asprintf(&shell_cmd, "fzf -f %s 2>/dev/null", shell_arg);
    menu->async_proc = async_proc_new(menu, 1, 0, _cmd_aproc_passthru_cb, shell_cmd);
    free(shell_arg);
    free(shell_cmd);
}

// Callback from cmd_grep
static int _cmd_grep_cb(cmd_context_t* ctx) {
    bint_t linenum;
    char* line;
    char* colon;
    line = strndup(ctx->bview->active_cursor->mark->bline->data, ctx->bview->active_cursor->mark->bline->data_len);
    colon = strchr(line, ':');
    if (colon == NULL) {
        free(line);
        return MLE_OK;
    }
    if (colon + 1 != '\0' && strchr(colon + 1, ':') != NULL) {
        linenum = strtoll(colon + 1, NULL, 10);
    } else {
        linenum = 0;
    }
    editor_close_bview(ctx->editor, ctx->bview, NULL);
    editor_open_bview(ctx->editor, NULL, MLE_BVIEW_TYPE_EDIT, line, (int)(colon - line), 1, linenum, &ctx->editor->rect_edit, NULL, NULL);
    free(line);
    return MLE_OK;
}

// Callback from cmd_browse
static int _cmd_browse_cb(cmd_context_t* ctx) {
    char* line;
    char* path;
    char* cwd;
    char* corrected_path;
    bview_t* new_bview;

    // Get path from tree output
    line = strndup(ctx->bview->active_cursor->mark->bline->data, ctx->bview->active_cursor->mark->bline->data_len);
    if ((path = strstr(line, "- ")) != NULL) {
        path += 2;
    } else if (strcmp(line, "..") == 0) {
        path = "..";
    } else if (util_is_dir(line)) {
        path = line;
    } else {
        MLE_SET_ERR(ctx->editor, "browse: Cannot browse to: '%s'", line);
        free(line);
        return MLE_ERR;
    }

    // Fix cwd if it changed
    cwd = get_current_dir_name();
    if (strcmp(cwd, ctx->bview->init_cwd) != 0) {
        asprintf(&corrected_path, "%s/%s", ctx->bview->init_cwd, path);
    } else {
        corrected_path = strdup(path);
    }

    // Open file or browse dir
    if (util_is_dir(corrected_path)) {
        chdir(corrected_path);
        ctx->bview = ctx->editor->active_edit;
        cmd_browse(ctx);
    } else {
        editor_open_bview(ctx->editor, NULL, MLE_BVIEW_TYPE_EDIT, corrected_path, strlen(corrected_path), 0, 0, &ctx->editor->rect_edit, NULL, &new_bview);
    }

    // Close menu
    editor_close_bview(ctx->editor, ctx->bview, NULL);

    // Set new_bview to active
    editor_set_active(ctx->editor, new_bview);

    free(corrected_path);
    free(line);
    return MLE_OK;
}
