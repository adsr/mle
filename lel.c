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
    char cparam;
    char* sparam;
    char* param1;
    char* param2;
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

static void _lel_func_cursor_swap_anchor(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_cursor_clone(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_cursor_collapse(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_cursor_drop_anchor(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_cursor_lift_anchor(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_cursor_wake(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_cursor_drop_mark(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_execute_group(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_filter_regex_condition(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_filter_regex_condition_not(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_filter_regex_replace(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_foreach_buffer(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_foreach_line(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_foreach_regex(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_foreach_regex_inverted(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_foreach_regex2(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_foreach_regex2_inverted(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_move_bracket(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_move_char(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_move_col(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_move_line(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_move_mark(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_move_orig(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_move_re(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_move_simple(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_move_str(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_move_word(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_register_append(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_register_clear(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_register_prepend(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_register_set(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_register_text_insert_after(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_register_text_insert_before(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_select(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_shell(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_shell_pipe(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_text_change(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_text_copy(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_text_copy_append(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_text_cut(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_text_delete(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_text_insert_after(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_text_insert_before(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_text_paste(lel_pnode_t* node, lel_ectx_t* ectx);
static void _lel_func_text_search_replace(lel_pnode_t* node, lel_ectx_t* ectx);
static char* _lel_get_sel(lel_pnode_t* node, lel_ectx_t* ectx);
static int _lel_compile_regex(char* re, pcre** ret_pcre);
static void _lel_execute(lel_pnode_t* tree, lel_ectx_t* ectx);
static lel_pnode_t* _lel_accept_cmd(lel_pstate_t* s);
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
static void _lel_free_node(lel_pnode_t* node);

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
    _lel_func_select,                        // :
    NULL,                                    // ;
    _lel_func_register_prepend,              // <
    _lel_func_register_set,                  // =
    _lel_func_register_append,               // >
    _lel_func_move_re,                       // ?
    NULL,                                    // @
    _lel_func_register_text_insert_before,   // A
    _lel_func_foreach_buffer,                // B
    NULL,                                    // C
    _lel_func_cursor_drop_anchor,            // D
    _lel_func_foreach_regex2_inverted,       // E
    _lel_func_move_str,                      // F
    _lel_func_move_simple,                   // G
    _lel_func_move_simple,                   // H
    _lel_func_register_text_insert_after,    // I
    NULL,                                    // J
    NULL,                                    // K
    _lel_func_foreach_line,                  // L
    _lel_func_cursor_drop_mark,              // M
    _lel_func_move_word,                     // N
    _lel_func_cursor_swap_anchor,            // O
    NULL,                                    // P
    _lel_func_filter_regex_condition_not,    // Q
    _lel_func_move_re,                       // R
    _lel_func_filter_regex_replace,          // S
    _lel_func_move_char,                     // T
    _lel_func_cursor_lift_anchor,            // U
    NULL,                                    // V
    _lel_func_move_word,                     // W
    _lel_func_foreach_regex_inverted,        // X
    _lel_func_text_copy_append,              // Y
    _lel_func_cursor_wake,                   // Z
    NULL,                                    // [
    NULL,                                    // backslash
    NULL,                                    // ]
    _lel_func_move_simple,                   // ^
    _lel_func_register_clear,                // _
    _lel_func_shell,                         // `
    _lel_func_text_insert_before,            // a
    _lel_func_move_bracket,                  // b
    _lel_func_text_change,                   // c
    _lel_func_text_delete,                   // d
    _lel_func_foreach_regex2,                // e
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
    _lel_func_move_char,                     // t
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
    lel_pstate_t lel_parse = {0};
    lel_ectx_t lel_ctx = {0};
    lel_pnode_t* lel_tree;

    if (ctx->static_param) {
        cmd = strdup(ctx->static_param);
    } else {
        editor_prompt(ctx->editor, "lel: Cmd?", NULL, &cmd);
        if (!cmd) return MLE_OK;
    }

    lel_parse.cmd = cmd;
    lel_parse.cmd_stop = cmd + strlen(cmd);
    lel_parse.cur = cmd;
    lel_tree = _lel_accept_cmds(&lel_parse);
    free(cmd);
    if (!lel_tree) return MLE_ERR;

    lel_ctx.ctx = ctx;
    bview_add_cursor(ctx->bview, ctx->cursor->mark->bline, ctx->cursor->mark->col, &lel_ctx.cursor);
    lel_ctx.bview = ctx->bview;
    lel_ctx.mark_start = buffer_add_mark(ctx->buffer, ctx->buffer->first_line, 0);
    lel_ctx.mark_end = buffer_add_mark(ctx->buffer, ctx->buffer->last_line, ctx->buffer->last_line->data_len);
    _lel_execute(lel_tree, &lel_ctx);
    _lel_free_node(lel_tree);
    buffer_destroy_mark(ctx->buffer, lel_ctx.mark_start);
    buffer_destroy_mark(ctx->buffer, lel_ctx.mark_end);
    bview_remove_cursor(ctx->bview, lel_ctx.cursor);

    return MLE_OK;
}

static void _lel_func_cursor_swap_anchor(lel_pnode_t* node, lel_ectx_t* ectx) {
    if (ectx->cursor->is_anchored) {
        mark_swap_with_mark(ectx->cursor->mark, ectx->cursor->anchor);
    }
}

static void _lel_func_cursor_clone(lel_pnode_t* node, lel_ectx_t* ectx) {
    bview_add_cursor_asleep(ectx->bview, ectx->cursor->mark->bline, ectx->cursor->mark->col, NULL);
}

static void _lel_func_cursor_collapse(lel_pnode_t* node, lel_ectx_t* ectx) {
    bview_remove_cursors_except(ectx->bview, ectx->cursor);
}

static void _lel_func_cursor_drop_anchor(lel_pnode_t* node, lel_ectx_t* ectx) {
    bview_cursor_drop_anchor(ectx->cursor);
}

static void _lel_func_cursor_lift_anchor(lel_pnode_t* node, lel_ectx_t* ectx) {
    bview_cursor_lift_anchor(ectx->cursor);
}

static void _lel_func_cursor_wake(lel_pnode_t* node, lel_ectx_t* ectx) {
    bview_wake_sleeping_cursors(ectx->bview);
}

static void _lel_func_cursor_drop_mark(lel_pnode_t* node, lel_ectx_t* ectx) {
    // TODO buffer_add_mark_with_letter
}

static void _lel_func_execute_group(lel_pnode_t* node, lel_ectx_t* ectx) {
    _lel_execute(node->child, ectx);
}

static void _lel_func_filter_regex_condition_inner(lel_pnode_t* node, lel_ectx_t* ectx, int negated) {
    char* sel;
    int rc;
    if (!_lel_compile_regex(node->param1, &node->re1)) return;
    sel = _lel_get_sel(node, ectx);
    rc = pcre_exec(node->re1, NULL, sel, strlen(sel), 0, 0, NULL, 0);
    if ((!negated && rc > 0) || (negated && rc < 0)) {
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

static void _lel_func_filter_regex_replace(lel_pnode_t* node, lel_ectx_t* ectx) {
    // TODO filter func
    //ctx->filter = fdsafds
    //_lel_execute(tree->child, ctx);
    //ctx->filter = orig;
    //return rv;
}

static void _lel_func_foreach_buffer(lel_pnode_t* node, lel_ectx_t* ectx) {
    // TODO
}

static void _lel_func_foreach_line(lel_pnode_t* node, lel_ectx_t* ectx) {
    bline_t* bline;
    mark_t* orig_cur;
    mark_t* orig_start;
    mark_t* orig_end;
    mark_clone(ectx->cursor->mark, &orig_cur);
    mark_clone(ectx->mark_start, &orig_start);
    mark_clone(ectx->mark_end, &orig_end);
    for (bline = ectx->mark_start->bline; bline && bline != ectx->mark_end->bline->next; bline = bline->next) {
        mark_move_to_w_bline(ectx->cursor->mark, bline, 0);
        mark_move_to_w_bline(ectx->mark_start, bline, 0);
        if (bline->next) {
            mark_move_to_w_bline(ectx->mark_end, bline->next, 0);
        } else {
            mark_move_to_w_bline(ectx->mark_end, bline, bline->data_len);
        }
        _lel_execute(node->child, ectx);
    }
    mark_join(ectx->cursor->mark, orig_cur);
    mark_join(ectx->mark_start, orig_start);
    mark_join(ectx->mark_end, orig_end);
    mark_destroy(orig_cur);
    mark_destroy(orig_start);
    mark_destroy(orig_end);
}

static void _lel_func_foreach_regex(lel_pnode_t* node, lel_ectx_t* ectx) {
    if (!_lel_compile_regex(node->param1, &node->re1)) return;

    //mark_move_next_cre

    // find regex matches from ctx->start to >end
    // foreach match {
    //   set ctx->start and >end
    //   for (node = tree->child; node; node = node->next) {
    //     node->func(node, &ctx);
    //   }
    // }
    // reset >start >end
}

static void _lel_func_foreach_regex_inverted(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_foreach_regex2(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_foreach_regex2_inverted(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_move_bracket(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_move_char(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_move_col(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_move_line(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_move_mark(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_move_orig(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_move_re(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_move_simple(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_move_str(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_move_word(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_register_append(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_register_clear(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_register_prepend(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_register_set(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_register_text_insert_after(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_register_text_insert_before(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_select(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_shell(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_shell_pipe(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_text_change(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_text_copy(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_text_copy_append(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_text_cut(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_text_delete(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_text_insert_after(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_text_insert_before(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_text_paste(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static void _lel_func_text_search_replace(lel_pnode_t* node, lel_ectx_t* ectx) {
}

static int _lel_compile_regex(char* re, pcre** ret_pcre) {
    return 0;
}

static char* _lel_get_sel(lel_pnode_t* node, lel_ectx_t* ectx) {
    return NULL;
}

static void _lel_execute(lel_pnode_t* tree, lel_ectx_t* ectx) {
    lel_pnode_t* node;
    lel_func_t func;
    for (node = tree; node; node = node->next) {
        func = node->ch <= '~' ? func_table[node->ch - '!'] : NULL;
        if (func) func(node, ectx);
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
        } else if ((ch = _lel_accept_any(s, "^$wWnNb~hHgGdkyYvDUOzZ.LB")) != NULL) {
            n.ch = *ch;
            if (n.ch == 'L' || n.ch == 'B') {
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
        } else if ((ch = _lel_accept_any(s, "rRfFaci|`xXqQ")) != NULL) {
            n.ch = *ch;
            _lel_expect_set_delim(s);
            n.param1 = _lel_expect_delim_str(s);
            _lel_expect_delim(s);
            if (n.ch == 'x' || n.ch == 'X' || n.ch == 'q' || n.ch == 'Q') {
                n.child = _lel_accept_cmd(s);
            }
        } else if ((ch = _lel_accept_any(s, "tTm:M><=_AI")) != NULL) {
            n.ch = *ch;
            n.cparam = _lel_expect_one(s);
        } else if ((ch = _lel_accept_any(s, "sSeE")) != NULL) {
            n.ch = *ch;
            _lel_expect_set_delim(s);
            n.param1 = _lel_expect_delim_str(s);
            _lel_expect_delim(s);
            n.param2 = _lel_expect_delim_str(s);
            _lel_expect_delim(s);
            if (n.ch != 's') {
                n.child = _lel_accept_cmd(s);
            }
        } else {
            has_node = 0;
        }
    } else {
        error = 1;
    }

    s->jmpbuf = jmpbuf_orig;

    if (error) {
        _lel_free_node(np);
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

static void _lel_free_node(lel_pnode_t* node) {
    if (node->param1) free(node->param1);
    if (node->param2) free(node->param2);
    if (node->child) _lel_free_node(node->child);
    if (node->next) _lel_free_node(node->next);
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
        return 1;
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
