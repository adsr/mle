#include <setjmp.h>
#include <ctype.h>
#include "mle.h"
#include "mlbuf.h"

typedef struct lel_pstate_s lel_pstate_t;
typedef struct lel_pnode_s lel_pnode_t;
typedef struct lel_ectx_s lel_ectx_t;
typedef void (*lel_func_t)(lel_pnode_t* tree, lel_ectx_t* ectx);

struct lel_pstate_s {
    char* cmd;
    char* cmd_stop;
    char* cur;
    char delim;
    jmp_buf* jmpbuf;
};

struct lel_pnode_s {
    char ch;
    char* sparam;
    char* param1;
    char* param2;
    char* param3;
    int num;
    int repeat;
    pcre* re1;
    pcre* re2;
    lel_pnode_t* child;
    lel_pnode_t* next;
};

struct lel_ectx_s {
    cmd_context_t* ctx;
    bview_t* bview;
    cursor_t* cursor;
    mark_t* orig;
    mark_t* mark_start;
    mark_t* mark_end;
};

static void _lel_takeover_first_active_cursor(cursor_t* main);
static void _lel_func_cursor_swap_anchor(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_cursor_clone(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_cursor_collapse(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_cursor_drop_anchor(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_cursor_lift_anchor(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_cursor_wake(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_cursor_drop_mark(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_execute_group(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_filter_regex_condition_inner(lel_pnode_t* node, lel_ectx_t* ectx, int negated);
static void _lel_func_filter_regex_condition(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_filter_regex_condition_not(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_foreach_line(lel_pnode_t* node, lel_ectx_t* ectx_orig);
static void _lel_func_foreach_regex(lel_pnode_t* node, lel_ectx_t* ectx_orig);
static void _lel_func_move_col(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_move_line(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_move_mark(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_move_orig(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_move_re(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_move_simple(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_move_str(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_move_word(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_register_mutate(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_register_write(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_shell_pipe(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_text_change(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_text_change_inner(lel_pnode_t* node, lel_ectx_t* ectx, int shell);
static void _lel_func_text_copy(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_text_copy_append(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_text_cut(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_text_insert_after(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_text_insert_before(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_text_paste(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_text_search_replace(lel_pnode_t* node, lel_ectx_t* ectx);
static lel_ectx_t* _lel_clone_context(lel_ectx_t* ectx_orig);
static void _lel_free_context(lel_ectx_t* ectx);
static int _lel_ensure_compiled_regex(char* re, pcre** ret_pcre);
static void _lel_get_sel_marks(lel_pnode_t* node, lel_ectx_t* ectx, mark_t** ret_a, mark_t** ret_b);
static char* _lel_get_sel(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_execute(lel_pnode_t* tree, lel_ectx_t* ectx);
static lel_pnode_t* _lel_accept_cmd(lel_pstate_t* s);
static void _lel_free_node(lel_pnode_t* node, int and_self);
static lel_pnode_t* _lel_accept_cmds(lel_pstate_t* s);
static char* _lel_accept_any(lel_pstate_t* s, char* any);
static int _lel_accept_num_inner(lel_pstate_t* s, int expect);
static int _lel_accept_num(lel_pstate_t* s);
static int _lel_expect_num(lel_pstate_t* s);
static void _lel_accept_ws(lel_pstate_t* s);
static int _lel_accept_inner(lel_pstate_t* s, char c, int expect);
static int _lel_accept(lel_pstate_t* s, char c);
static int _lel_expect(lel_pstate_t* s, char c);
static void _lel_expect_delim(lel_pstate_t* s);
static char _lel_expect_one(lel_pstate_t* s);
static void _lel_expect_set_delim(lel_pstate_t* s);
static char* _lel_accept_any_set_delim(lel_pstate_t* s, char* any);
static char* _lel_expect_delim_str(lel_pstate_t* s);

static lel_func_t func_table[] = {
    NULL,                                    // !
    _lel_func_move_str,                      // "
    _lel_func_move_col,                      // #
    _lel_func_move_simple,                   // $
    NULL,                                    // %
    NULL,                                    // &
    _lel_func_move_str,                      // '
    NULL,                                    // (
    NULL,                                    // )
    NULL,                                    // *
    NULL,                                    // +
    NULL,                                    // ,
    NULL,                                    // -
    _lel_func_cursor_collapse,               // .
    _lel_func_move_re,                       // /
    NULL,                                    // 0
    NULL,                                    // 1
    NULL,                                    // 2
    NULL,                                    // 3
    NULL,                                    // 4
    NULL,                                    // 5
    NULL,                                    // 6
    NULL,                                    // 7
    NULL,                                    // 8
    NULL,                                    // 9
    NULL,                                    // :
    NULL,                                    // ;
    _lel_func_register_mutate,               // <
    _lel_func_register_mutate,               // =
    _lel_func_register_mutate,               // >
    _lel_func_move_re,                       // ?
    NULL,                                    // @
    _lel_func_register_write,                // A
    NULL,                                    // B
    NULL,                                    // C
    _lel_func_cursor_drop_anchor,            // D
    NULL,                                    // E
    _lel_func_move_str,                      // F
    _lel_func_move_simple,                   // G
    _lel_func_move_simple,                   // H
    _lel_func_register_write,                // I
    NULL,                                    // J
    NULL,                                    // K
    _lel_func_foreach_line,                  // L
    _lel_func_cursor_drop_mark,              // M
    _lel_func_move_word,                     // N
    _lel_func_cursor_swap_anchor,            // O
    NULL,                                    // P
    _lel_func_filter_regex_condition_not,    // Q
    _lel_func_move_re,                       // R
    _lel_func_register_write,                // S
    _lel_func_move_str,                      // T
    _lel_func_cursor_lift_anchor,            // U
    NULL,                                    // V
    _lel_func_move_word,                     // W
    _lel_func_foreach_regex,                 // X
    _lel_func_text_copy_append,              // Y
    _lel_func_cursor_wake,                   // Z
    NULL,                                    // [
    NULL,                                    // backslash
    NULL,                                    // ]
    _lel_func_move_simple,                   // ^
    _lel_func_register_mutate,               // _
    NULL,                                    // `
    _lel_func_text_insert_before,            // a
    NULL,                                    // b
    _lel_func_text_change,                   // c
    _lel_func_text_change,                   // d
    NULL,                                    // e
    _lel_func_move_str,                      // f
    _lel_func_move_simple,                   // g
    _lel_func_move_simple,                   // h
    _lel_func_text_insert_after,             // i
    NULL,                                    // j
    _lel_func_text_cut,                      // k
    _lel_func_move_line,                     // l
    _lel_func_move_mark,                     // m
    _lel_func_move_word,                     // n
    NULL,                                    // o
    NULL,                                    // p
    _lel_func_filter_regex_condition,        // q
    _lel_func_move_re,                       // r
    _lel_func_text_search_replace,           // s
    _lel_func_move_str,                      // t
    NULL,                                    // u
    _lel_func_text_paste,                    // v
    _lel_func_move_word,                     // w
    _lel_func_foreach_regex,                 // x
    _lel_func_text_copy,                     // y
    _lel_func_cursor_clone,                  // z
    _lel_func_execute_group,                 // {
    _lel_func_shell_pipe,                    // |
    NULL,                                    // }
    _lel_func_move_orig,                     // ~
};

int cmd_lel(cmd_context_t* ctx) {
    char* cmd;
    int cursor_count;
    lel_pstate_t lel_parse = {0};
    lel_ectx_t lel_ctx = {0};
    lel_pnode_t* lel_tree;

    // Ask for lel cmd if not present as static param
    if (ctx->static_param) {
        cmd = strdup(ctx->static_param);
    } else {
        editor_prompt(ctx->editor, "lel: Cmd?", NULL, &cmd);
        if (!cmd) return MLE_OK;
    }

    // Parse lel cmd
    lel_parse.cmd = cmd;
    lel_parse.cmd_stop = cmd + strlen(cmd);
    lel_parse.cur = cmd;
    lel_tree = _lel_accept_cmds(&lel_parse);
    free(cmd);
    if (!lel_tree) return MLE_ERR;

    // Setup a cursor to use for the lel program
    lel_ctx.ctx = ctx;
    bview_add_cursor(ctx->bview, ctx->cursor->mark->bline, ctx->cursor->mark->col, &lel_ctx.cursor);
    lel_ctx.bview = ctx->bview;
    mark_clone(ctx->cursor->mark, &lel_ctx.orig);
    lel_ctx.mark_start = buffer_add_mark(ctx->buffer, ctx->buffer->first_line, 0);
    lel_ctx.mark_end = buffer_add_mark(ctx->buffer, ctx->buffer->last_line, ctx->buffer->last_line->data_len);
    if (ctx->cursor->is_anchored) {
        if (mark_is_lt(ctx->cursor->mark, ctx->cursor->anchor)) {
            mark_join(lel_ctx.mark_start, ctx->cursor->mark);
            mark_join(lel_ctx.mark_end, ctx->cursor->anchor);
        } else {
            mark_join(lel_ctx.mark_start, ctx->cursor->anchor);
            mark_join(lel_ctx.mark_end, ctx->cursor->mark);
        }
    }

    // Remember how many active cursors there were
    cursor_count = bview_get_active_cursor_count(ctx->bview);

    // Run the lel cmd
    _lel_execute(lel_tree, &lel_ctx);

    // If the active cursor count increased, let the main cursor takeover the
    // first other active cursor. This is a convenience measure so that the lel
    // cmd does not need to care about deduplicating the main cursor. E.g.,
    // consider the lel cmd `X/string/z Z` which drops an active cursor on every
    // term "string". If we didn't do this takeover, the main cursor would still
    // be floating around, preventing us from properly editing all "string"
    // matches.
    if (bview_get_active_cursor_count(ctx->bview) - cursor_count > 0) {
        _lel_takeover_first_active_cursor(ctx->cursor);
    }

    // Deallocate stuff
    _lel_free_node(lel_tree, 1);
    mark_destroy(lel_ctx.mark_start);
    mark_destroy(lel_ctx.mark_end);
    mark_destroy(lel_ctx.orig);
    bview_remove_cursor(ctx->bview, lel_ctx.cursor);

    return MLE_OK;
}

static void _lel_takeover_first_active_cursor(cursor_t* main) {
    cursor_t* cursor;
    cursor_t* cursor_tmp;
    DL_FOREACH_SAFE(main->bview->cursors, cursor, cursor_tmp) {
        if (cursor == main || cursor->is_asleep) continue;
        mark_join(main->mark, cursor->mark);
        if (cursor->is_anchored) {
            cursor_drop_anchor(main, cursor->sel_rule ? 1 : 0);
            mark_join(main->anchor, cursor->anchor);
        }
        cursor_destroy(cursor);
        return;
    }
}

static lel_pnode_t* _lel_accept_cmd(lel_pstate_t* s) {
    lel_pnode_t n = {0};
    jmp_buf jmpbuf = {0};
    jmp_buf* jmpbuf_orig;
    int error;
    int has_node;
    lel_pnode_t* np;
    char* ch;
    bint_t num;

    error = 0;
    has_node = 1;
    np = &n;

    jmpbuf_orig = s->jmpbuf;
    s->jmpbuf = &jmpbuf;

    if (!setjmp(jmpbuf)) {
        _lel_accept_ws(s);
        n.repeat = _lel_accept_num(s);
        _lel_accept_ws(s);
        if (_lel_accept(s, '{')) {
            n.ch = '{';
            n.child = _lel_accept_cmds(s);
            _lel_expect(s, '}');
        } else if ((ch = _lel_accept_any(s, "^$wWnN~hHgGdkyYvDUOzZ.L")) != NULL) {
            n.ch = *ch;
            if (n.ch == 'L') {
                n.child = _lel_accept_cmd(s);
            }
        } else if ((ch = _lel_accept_any_set_delim(s, "/?\"'")) != NULL) {
            n.ch = *ch;
            n.param1 = _lel_expect_delim_str(s);
            _lel_expect_delim(s);
        } else if ((ch = _lel_accept_any(s, "l#")) != NULL) {
            n.ch = *ch;
            if ((ch = _lel_accept_any(s, "+-")) != NULL) {
                num = *ch == '-' ? -1 : 1;
                n.sparam = "rel";
            } else {
                num = 1;
                n.sparam = "abs";
            }
            n.num = num * _lel_expect_num(s);
        } else if ((ch = _lel_accept_any(s, "rRfFaci|xXqQ")) != NULL) {
            n.ch = *ch;
            _lel_expect_set_delim(s);
            n.param1 = _lel_expect_delim_str(s);
            _lel_expect_delim(s);
            if (n.ch == 'x' || n.ch == 'X' || n.ch == 'q' || n.ch == 'Q') {
                n.child = _lel_accept_cmd(s);
            }
        } else if ((ch = _lel_accept_any(s, "tTmM><=_AI")) != NULL) {
            n.ch = *ch;
            n.param1 = malloc(2);
            n.param1[0] = _lel_expect_one(s);
            n.param1[1] = '\0';
        } else if ((ch = _lel_accept_any(s, "s")) != NULL) {
            n.ch = *ch;
            _lel_expect_set_delim(s);
            n.param1 = _lel_expect_delim_str(s);
            _lel_expect_delim(s);
            n.param2 = _lel_expect_delim_str(s);
            _lel_expect_delim(s);
        } else if ((ch = _lel_accept_any(s, "S")) != NULL) {
            n.ch = *ch;
            n.param1 = malloc(2);
            n.param1[0] = _lel_expect_one(s);
            n.param1[1] = '\0';
            _lel_expect_set_delim(s);
            n.param2 = _lel_expect_delim_str(s);
            _lel_expect_delim(s);
            n.param3 = _lel_expect_delim_str(s);
            _lel_expect_delim(s);
        } else {
            has_node = 0;
        }
    } else {
        error = 1;
    }

    s->jmpbuf = jmpbuf_orig;

    if (error) {
        _lel_free_node(np, 0);
        np = NULL;
        if (s->jmpbuf) longjmp(*s->jmpbuf, 1);
    } else if (!has_node) {
        np = NULL;
    } else {
        np = malloc(sizeof(lel_pnode_t));
        *np = n;
    }

    return np;
}

static void _lel_func_cursor_swap_anchor(lel_pnode_t* node, lel_ectx_t* ectx) {
    if (ectx->cursor->is_anchored) {
        mark_swap_with_mark(ectx->cursor->mark, ectx->cursor->anchor);
    }
}

static void _lel_func_cursor_clone(lel_pnode_t* node, lel_ectx_t* ectx) {
    cursor_t* clone;
    cursor_clone(ectx->cursor, 1, &clone);
    clone->is_asleep = 1;
}

static void _lel_func_cursor_collapse(lel_pnode_t* node, lel_ectx_t* ectx) {
    cursor_t* cursor;
    DL_FOREACH(ectx->cursor->bview->cursors, cursor) {
        if (cursor != ectx->cursor && cursor != ectx->ctx->cursor) {
            bview_remove_cursor(ectx->cursor->bview, cursor);
        }
    }
}

static void _lel_func_cursor_drop_anchor(lel_pnode_t* node, lel_ectx_t* ectx) {
    cursor_drop_anchor(ectx->cursor, 1);
}

static void _lel_func_cursor_lift_anchor(lel_pnode_t* node, lel_ectx_t* ectx) {
    cursor_lift_anchor(ectx->cursor);
}

static void _lel_func_cursor_wake(lel_pnode_t* node, lel_ectx_t* ectx) {
    bview_wake_sleeping_cursors(ectx->bview);
}

static void _lel_func_cursor_drop_mark(lel_pnode_t* node, lel_ectx_t* ectx) {
    buffer_add_lettered_mark(ectx->cursor->mark->bline->buffer, *node->param1, ectx->cursor->mark->bline, ectx->cursor->mark->col);
}

static void _lel_func_execute_group(lel_pnode_t* node, lel_ectx_t* ectx) {
    _lel_execute(node->child, ectx);
}

static void _lel_func_filter_regex_condition_inner(lel_pnode_t* node, lel_ectx_t* ectx, int negated) {
    char* sel;
    int rc;
    if (!_lel_ensure_compiled_regex(node->param1, &node->re1)) return;
    sel = _lel_get_sel(node, ectx);
    rc = pcre_exec(node->re1, NULL, sel, strlen(sel), 0, 0, NULL, 0);
    if ((!negated && rc >= 0) || (negated && rc < 0)) {
        _lel_execute(node->child, ectx);
    }
    free(sel);
}

static void _lel_func_filter_regex_condition(lel_pnode_t* node, lel_ectx_t* ectx) {
    _lel_func_filter_regex_condition_inner(node, ectx, 0);
}

static void _lel_func_filter_regex_condition_not(lel_pnode_t* node, lel_ectx_t* ectx) {
    _lel_func_filter_regex_condition_inner(node, ectx, 1);
}

static void _lel_func_foreach_line(lel_pnode_t* node, lel_ectx_t* ectx_orig) {
    lel_ectx_t* ectx;
    bline_t* bline;
    bint_t num_lines;

    ectx = _lel_clone_context(ectx_orig);
    num_lines = ectx->mark_end->bline->line_index - ectx->mark_start->bline->line_index + 1;
    bline = ectx->mark_start->bline;
    while (bline && num_lines > 0) {
        mark_move_to_w_bline(ectx->cursor->mark, bline, 0);
        mark_move_to_w_bline(ectx->mark_start, bline, 0);
        if (bline->next) {
            mark_move_to_w_bline(ectx->mark_end, bline->next, 0);
        } else {
            mark_move_to_w_bline(ectx->mark_end, bline, bline->data_len);
        }
        _lel_execute(node->child, ectx);
        num_lines -= 1;
        bline = ectx->mark_start->bline->next;
    }
    _lel_free_context(ectx);
}

static void _lel_func_foreach_regex(lel_pnode_t* node, lel_ectx_t* ectx_orig) {
    lel_ectx_t* ectx;
    bint_t num_chars;
    int orig_budge;
    if (!_lel_ensure_compiled_regex(node->param1, &node->re1)) return;
    ectx = _lel_clone_context(ectx_orig);
    mark_join(ectx->cursor->mark, ectx->mark_start);
    while (mark_is_lte(ectx->cursor->mark, ectx_orig->mark_end)) {
        mark_set_find_budge(0, &orig_budge);
        if (mark_move_next_cre_ex(ectx->cursor->mark, node->re1, NULL, NULL, &num_chars) != MLBUF_OK) {
            break;
        } else if (mark_is_gt(ectx->cursor->mark, ectx_orig->mark_end)) {
            break;
        }
        mark_set_find_budge(orig_budge, NULL);
        mark_join(ectx->mark_start, ectx->cursor->mark);
        mark_join(ectx->mark_end, ectx->cursor->mark);
        mark_move_by(ectx->mark_end, num_chars);
        if (node->ch == 'X') {
            cursor_select_between(ectx->cursor, ectx->mark_start, ectx->mark_end, 0);
        }
        _lel_execute(node->child, ectx);
        mark_join(ectx->cursor->mark, ectx->mark_end);
    }
    _lel_free_context(ectx);
}

static void _lel_func_move_col(lel_pnode_t* node, lel_ectx_t* ectx) {
    if (!node->sparam) return;
    if (strcmp(node->sparam, "rel") == 0) {
        mark_move_by(ectx->cursor->mark, node->num);
    } else if (strcmp(node->sparam, "abs") == 0) {
        mark_move_col(ectx->cursor->mark, node->num);
    }
}

static void _lel_func_move_line(lel_pnode_t* node, lel_ectx_t* ectx) {
    if (!node->sparam) return;
    if (strcmp(node->sparam, "rel") == 0) {
        mark_move_vert(ectx->cursor->mark, node->num);
        mark_move_bol(ectx->cursor->mark);
    } else if (strcmp(node->sparam, "abs") == 0) {
        mark_move_to(ectx->cursor->mark, node->num, 0);
    }
}

static void _lel_func_move_mark(lel_pnode_t* node, lel_ectx_t* ectx) {
    mark_t* mark;
    mark = NULL;
    buffer_get_lettered_mark(ectx->cursor->mark->bline->buffer, *node->param1, &mark);
    if (mark) mark_join(ectx->cursor->mark, mark);
}

static void _lel_func_move_orig(lel_pnode_t* node, lel_ectx_t* ectx) {
    mark_join(ectx->cursor->mark, ectx->orig);
}

static void _lel_func_move_re(lel_pnode_t* node, lel_ectx_t* ectx) {
    if (!_lel_ensure_compiled_regex(node->param1, &node->re1)) return;
    if (strchr("/r", node->ch)) {
        mark_move_next_cre(ectx->cursor->mark, node->re1);
    } else if (strchr("?R", node->ch)) {
        mark_move_prev_cre(ectx->cursor->mark, node->re1);
    }
}

static void _lel_func_move_simple(lel_pnode_t* node, lel_ectx_t* ectx) {
    if (node->ch == '^') {
        mark_move_bol(ectx->cursor->mark);
    } else if (node->ch == '$') {
        mark_move_eol(ectx->cursor->mark);
    } else if (node->ch == 'g') {
        mark_move_beginning(ectx->cursor->mark);
    } else if (node->ch == 'G') {
        mark_move_end(ectx->cursor->mark);
    } else if (node->ch == 'h') {
        mark_join(ectx->cursor->mark, ectx->mark_start);
    } else if (node->ch == 'H') {
        mark_join(ectx->cursor->mark, ectx->mark_end);
    }
}

static void _lel_func_move_str(lel_pnode_t* node, lel_ectx_t* ectx) {
    if (strchr("'ft", node->ch)) {
        mark_move_next_str(ectx->cursor->mark, node->param1, strlen(node->param1));
    } else if (strchr("\"FT", node->ch)) {
        mark_move_prev_str(ectx->cursor->mark, node->param1, strlen(node->param1));
    }
}

static void _lel_func_move_word(lel_pnode_t* node, lel_ectx_t* ectx) {
    if (!node->re1) {
        if (node->ch == 'w') {
            _lel_ensure_compiled_regex("(?<=\\w)(\\W|$)", &node->re1);
        } else if (node->ch == 'n') {
            _lel_ensure_compiled_regex("(\\w|$)(?=(\\W|$))", &node->re1);
        } else if (node->ch == 'W') {
            _lel_ensure_compiled_regex("\\W(?=\\w|$)", &node->re1);
        } else if (node->ch == 'N') {
            _lel_ensure_compiled_regex("(?<=\\W|^)(\\w|^)", &node->re1);
        }
    }
    if (!node->re1) return;
    if (node->ch == 'w' || node->ch == 'n') {
        mark_move_next_cre(ectx->cursor->mark, node->re1);
    } else if (node->ch == 'W' || node->ch == 'N') {
        mark_move_prev_cre(ectx->cursor->mark, node->re1);
    }
}

static void _lel_func_register_mutate(lel_pnode_t* node, lel_ectx_t* ectx) {
    char* sel;
    int (*regfunc)(buffer_t*, char, char*, size_t);
    if (node->ch == '>') {
        regfunc = buffer_register_append;
    } else if (node->ch == '<') {
        regfunc = buffer_register_prepend;
    } else if (node->ch == '=') {
        regfunc = buffer_register_set;
    } else if (node->ch == '_') {
        buffer_register_clear(ectx->cursor->mark->bline->buffer, *node->param1);
        return;
    } else {
        return;
    }
    sel = _lel_get_sel(node, ectx);
    regfunc(ectx->cursor->mark->bline->buffer, *node->param1, sel, strlen(sel));
}

static void _lel_func_register_write(lel_pnode_t* node, lel_ectx_t* ectx) {
    char* data;
    char* data_repl;
    int data_repl_len;
    size_t data_len;
    int (*markfn)(mark_t*, char*, bint_t);
    if (buffer_register_get(ectx->cursor->mark->bline->buffer, *node->param1, 0, &data, &data_len) != MLBUF_OK) {
        return;
    }
    data_repl = NULL;
    if (node->ch == 'A') {
        markfn = mark_insert_before;
    } else if (node->ch == 'I') {
        markfn = mark_insert_after;
    } else if (node->ch == 'S') {
        markfn = mark_insert_before;
        util_pcre_replace(node->param2, data, node->param3, &data_repl, &data_repl_len);
        data = data_repl;
        data_len = data_repl_len;
    } else {
        return;
    }
    markfn(ectx->cursor->mark, data, (bint_t)data_len);
    if (data_repl) free(data_repl);
}

static void _lel_func_shell_pipe(lel_pnode_t* node, lel_ectx_t* ectx) {
    _lel_func_text_change_inner(node, ectx, 1);
}

static void _lel_func_text_change(lel_pnode_t* node, lel_ectx_t* ectx) {
    _lel_func_text_change_inner(node, ectx, 0);
}

static void _lel_func_text_change_inner(lel_pnode_t* node, lel_ectx_t* ectx, int shell) {
    mark_t* mark_a;
    mark_t* mark_b;
    char* shell_in;
    char* shell_out;
    bint_t shell_in_len;
    size_t shell_out_len;
    char* repl;
    bint_t repl_len;

    repl = NULL;
    repl_len = 0;
    shell_in = NULL;
    shell_out = NULL;
    _lel_get_sel_marks(node, ectx, &mark_a, &mark_b);

    if (shell) {
        mark_get_between_mark(mark_a, mark_b, &shell_in, &shell_in_len);
        if (shell_in_len > 0
            && util_shell_exec(ectx->ctx->editor, node->param1, 1, shell_in, (size_t)shell_in_len, NULL, &shell_out, &shell_out_len) == MLE_OK
            && shell_out_len > 0
        ) {
            repl = shell_out;
            repl_len = shell_out_len;
        }
    } else if (node->param1) {
        if (ectx->cursor->is_anchored) {
            repl = strdup(node->param1);
        } else {
            asprintf(&repl, "%s\n", node->param1);
        }
        repl_len = strlen(repl);
    }

    mark_replace_between_mark(mark_a, mark_b, repl, repl_len);

    if (shell_in) free(shell_in);
    if (repl) free(repl);
    mark_destroy(mark_a);
    mark_destroy(mark_b);
}

static void _lel_func_text_copy(lel_pnode_t* node, lel_ectx_t* ectx) {
    cursor_cut_copy(ectx->cursor, 0, 0, 0);
}

static void _lel_func_text_copy_append(lel_pnode_t* node, lel_ectx_t* ectx) {
    cursor_cut_copy(ectx->cursor, 0, 0, 1);
}

static void _lel_func_text_cut(lel_pnode_t* node, lel_ectx_t* ectx) {
    cursor_cut_copy(ectx->cursor, 1, 0, 0);
}

static void _lel_func_text_insert_after(lel_pnode_t* node, lel_ectx_t* ectx) {
    mark_insert_after(ectx->cursor->mark, node->param1, (bint_t)strlen(node->param1));
}

static void _lel_func_text_insert_before(lel_pnode_t* node, lel_ectx_t* ectx) {
    mark_insert_before(ectx->cursor->mark, node->param1, (bint_t)strlen(node->param1));
}

static void _lel_func_text_paste(lel_pnode_t* node, lel_ectx_t* ectx) {
    cursor_uncut(ectx->cursor);
}

static void _lel_func_text_search_replace(lel_pnode_t* node, lel_ectx_t* ectx) {
    cursor_t* cursor;
    mark_t* mark_a;
    mark_t* mark_b;
    cursor_clone(ectx->cursor, 0, &cursor);
    _lel_get_sel_marks(node, ectx, &mark_a, &mark_b);
    cursor_drop_anchor(cursor, 0);
    mark_join(cursor->mark, mark_a);
    mark_join(cursor->anchor, mark_b);
    cursor_replace(cursor, 0, node->param1, node->param2);
    mark_destroy(mark_a);
    mark_destroy(mark_b);
    cursor_destroy(cursor);
}

static lel_ectx_t* _lel_clone_context(lel_ectx_t* ectx_orig) {
    lel_ectx_t* ectx;
    ectx = malloc(sizeof(lel_ectx_t));
    *ectx = *ectx_orig;
    mark_clone(ectx_orig->mark_start, &ectx->mark_start);
    mark_clone(ectx_orig->mark_end, &ectx->mark_end);
    return ectx;
}

static void _lel_free_context(lel_ectx_t* ectx) {
    mark_destroy(ectx->mark_start);
    mark_destroy(ectx->mark_end);
    free(ectx);
}

static int _lel_ensure_compiled_regex(char* re, pcre** ret_pcre) {
    pcre* cre;
    const char *error;
    int erroffset;
    if (*ret_pcre) return 1;
    cre = pcre_compile((const char*)re, PCRE_NO_AUTO_CAPTURE | PCRE_CASELESS, &error, &erroffset, NULL);
    if (!cre) {
        *ret_pcre = NULL;
        return 0;
    }
    *ret_pcre = cre;
    return 1;
}

static void _lel_get_sel_marks(lel_pnode_t* node, lel_ectx_t* ectx, mark_t** ret_a, mark_t** ret_b) {
    if (ectx->cursor->is_anchored) {
        if (mark_is_lt(ectx->cursor->mark, ectx->cursor->anchor)) {
            mark_clone(ectx->cursor->mark, ret_a);
            mark_clone(ectx->cursor->anchor, ret_b);
        } else {
            mark_clone(ectx->cursor->mark, ret_b);
            mark_clone(ectx->cursor->anchor, ret_a);
        }
    } else {
        mark_clone(ectx->cursor->mark, ret_a);
        mark_clone(ectx->cursor->mark, ret_b);
        mark_move_bol(*ret_a);
        mark_move_eol(*ret_b);
        mark_move_by(*ret_b, 1);
    }
}

static char* _lel_get_sel(lel_pnode_t* node, lel_ectx_t* ectx) {
    char *sel;
    bint_t sel_len;
    mark_t* mark_a;
    mark_t* mark_b;
    _lel_get_sel_marks(node, ectx, &mark_a, &mark_b);
    mark_get_between_mark(mark_a, mark_b, &sel, &sel_len);
    mark_destroy(mark_a);
    mark_destroy(mark_b);
    return sel;
}

static void _lel_execute(lel_pnode_t* tree, lel_ectx_t* ectx) {
    lel_pnode_t* node;
    lel_func_t func;
    int i;
    for (node = tree; node; node = node->next) {
        func = node->ch >= '!' && node->ch <= '~' ? func_table[node->ch - '!'] : NULL;
        if (func) for (i = 0; i < node->repeat; i++) func(node, ectx);
    }
}

static void _lel_free_node(lel_pnode_t* node, int and_self) {
    if (node->param1) free(node->param1);
    if (node->param2) free(node->param2);
    if (node->param3) free(node->param3);
    if (node->re1) pcre_free(node->re1);
    if (node->re2) pcre_free(node->re2);
    if (node->child) _lel_free_node(node->child, 1);
    if (node->next) _lel_free_node(node->next, 1);
    if (and_self) free(node);
}

static lel_pnode_t* _lel_accept_cmds(lel_pstate_t* s) {
    lel_pnode_t* head;
    lel_pnode_t* prev;
    lel_pnode_t* node;
    head = NULL;
    prev = NULL;
    while ((node = _lel_accept_cmd(s)) != NULL) {
        if (!head) head = node;
        if (prev) prev->next = node;
        prev = node;
    }
    return head;
}

static char* _lel_accept_any(lel_pstate_t* s, char* any) {
    char* ch;
    if (s->cur >= s->cmd_stop) return NULL;
    ch = strchr(any, *s->cur);
    if (ch) s->cur++;
    return ch;
}

static int _lel_accept_num_inner(lel_pstate_t* s, int expect) {
    char* digit;
    int num;
    digit = _lel_accept_any(s, "0123456789");
    if (!digit) {
        if (expect) longjmp(*s->jmpbuf, 1);
        return 1; // 1 is the default number
    }
    num = (int)(*digit - '0');
    while ((digit = _lel_accept_any(s, "0123456789")) != NULL) {
        num = (num * 10) + (int)(*digit - '0');
    }
    return num;
}

static int _lel_accept_num(lel_pstate_t* s) {
    return _lel_accept_num_inner(s, 0);
}

static int _lel_expect_num(lel_pstate_t* s) {
    return _lel_accept_num_inner(s, 1);
}

static void _lel_accept_ws(lel_pstate_t* s) {
    while (s->cur < s->cmd_stop && isspace(*s->cur)) {
        s->cur++;
    }
}

static int _lel_accept_inner(lel_pstate_t* s, char c, int expect) {
    char* ch;
    char str[2];
    str[0] = c;
    str[1] = '\0';
    ch = _lel_accept_any(s, str);
    if (ch) {
        return 1;
    } else if (expect) {
        longjmp(*s->jmpbuf, 1);
    }
    return 0;
}

static int _lel_accept(lel_pstate_t* s, char c) {
    return _lel_accept_inner(s, c, 0);
}

static int _lel_expect(lel_pstate_t* s, char c) {
    return _lel_accept_inner(s, c, 1);
}

static void _lel_expect_delim(lel_pstate_t* s) {
    if (s->cur >= s->cmd_stop || *s->cur != s->delim) {
        longjmp(*s->jmpbuf, 1);
    }
    s->cur++;
}

static char _lel_expect_one(lel_pstate_t* s) {
    char ch;
    if (s->cur >= s->cmd_stop) longjmp(*s->jmpbuf, 1);
    ch = *s->cur;
    s->cur++;
    return ch;
}

static void _lel_expect_set_delim(lel_pstate_t* s) {
    s->delim = _lel_expect_one(s);
}

static char* _lel_accept_any_set_delim(lel_pstate_t* s, char* any) {
    char* ch = _lel_accept_any(s, any);
    if (ch) s->delim = *ch;
    return ch;
}

static char* _lel_expect_delim_str(lel_pstate_t* s) {
    char* str;
    int len;
    if (s->cur >= s->cmd_stop) longjmp(*s->jmpbuf, 1);
    len = 0;
    str = malloc((s->cmd_stop - s->cur + 1) * sizeof(char));
    while (s->cur < s->cmd_stop && *s->cur != s->delim) {
        if (*s->cur == '\\'
            && s->cur+1 < s->cmd_stop
            && (*(s->cur+1) == 'n' || *(s->cur+1) == s->delim)
        ) {
            str[len++] = (*(s->cur+1) == 'n' ? '\n' : s->delim);
            s->cur += 2;
        } else {
            str[len++] = *s->cur;
            s->cur += 1;
        }
    }
    str[len] = '\0';
    return str;
}
