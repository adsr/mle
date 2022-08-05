#include <unistd.h>
#include <signal.h>
#include <uthash.h>
#include <utlist.h>
#include <inttypes.h>
#include <fnmatch.h>
#define TB_IMPL
#include "termbox2.h"
#undef TB_IMPL
#include "mle.h"
#include "mlbuf.h"

static int _editor_set_macro_toggle_key(editor_t *editor, char *key);
static int _editor_bview_exists(editor_t *editor, bview_t *bview);
static int _editor_register_cmd_fn(editor_t *editor, char *name, int (*func)(cmd_context_t *ctx));
static int _editor_should_skip_rc(char **argv);
static int _editor_close_bview_inner(editor_t *editor, bview_t *bview, int *optret_num_closed);
static int _editor_destroy_cmd(editor_t *editor, cmd_t *cmd);
static int _editor_prompt_input_submit(cmd_context_t *ctx);
static int _editor_prompt_input_complete(cmd_context_t *ctx);
static prompt_history_t *_editor_prompt_find_or_add_history(cmd_context_t *ctx, prompt_hnode_t **optret_prompt_hnode);
static int _editor_prompt_history_up(cmd_context_t *ctx);
static int _editor_prompt_history_down(cmd_context_t *ctx);
static int _editor_prompt_history_append(cmd_context_t *ctx, char *data);
static int _editor_prompt_yna_all(cmd_context_t *ctx);
static int _editor_prompt_yn_yes(cmd_context_t *ctx);
static int _editor_prompt_yn_no(cmd_context_t *ctx);
static int _editor_prompt_cancel(cmd_context_t *ctx);
static int _editor_menu_submit(cmd_context_t *ctx);
static int _editor_menu_cancel(cmd_context_t *ctx);
static int _editor_prompt_isearch_next(cmd_context_t *ctx);
static int _editor_prompt_isearch_prev(cmd_context_t *ctx);
static int _editor_prompt_isearch_viewport_up(cmd_context_t *ctx);
static int _editor_prompt_isearch_viewport_down(cmd_context_t *ctx);
static int _editor_prompt_isearch_drop_cursors(cmd_context_t *ctx);
static void _editor_loop(editor_t *editor, loop_context_t *loop_ctx);
static void _editor_refresh_cmd_context(editor_t *editor, cmd_context_t *cmd_ctx);
static void _editor_notify_cmd_observers(cmd_context_t *ctx, int is_before);
static int _editor_maybe_toggle_macro(editor_t *editor, kinput_t *input);
static void _editor_resize(editor_t *editor, int w, int h);
static void _editor_maybe_lift_temp_anchors(cmd_context_t *ctx);
static void _editor_draw_cursors(editor_t *editor, bview_t *bview);
static void _editor_get_user_input(editor_t *editor, cmd_context_t *ctx);
static void _editor_ingest_paste(editor_t *editor, cmd_context_t *ctx);
static void _editor_append_pastebuf(editor_t *editor, cmd_context_t *ctx, kinput_t *input);
static void _editor_record_macro_input(kmacro_t *macro, kinput_t *input);
static cmd_t *_editor_get_command(editor_t *editor, cmd_context_t *ctx, kinput_t *opt_paste_input);
static kbinding_t *_editor_get_kbinding_node(kbinding_t *node, kinput_t *input, cmd_context_t *ctx, int is_paste, int *ret_is_numeric);
static cmd_t *_editor_resolve_cmd(editor_t *editor, cmd_t **rcmd, char *cmd_name);
static int _editor_key_to_input(char *key, kinput_t *ret_input);
static void _editor_init_signal_handlers(editor_t *editor);
static void _editor_continue(int signum);
static void _editor_graceful_exit(int signum);
static void _editor_register_cmds(editor_t *editor);
static void _editor_init_kmaps(editor_t *editor);
static void _editor_init_kmap(editor_t *editor, kmap_t **ret_kmap, char *name, char *default_cmd_name, int allow_fallthru, kbinding_def_t *defs);
static void _editor_init_kmap_add_binding(editor_t *editor, kmap_t *kmap, kbinding_def_t *binding_def);
static int _editor_init_kmap_add_binding_to_trie(kbinding_t **trie, char *cmd_name, char *cur_key_patt, char *full_key_patt, char *static_param);
static int _editor_init_kmap_by_str(editor_t *editor, kmap_t **ret_kmap, char *str);
static int _editor_init_kmap_add_binding_by_str(editor_t *editor, kmap_t *kmap, char *str);
static void _editor_destroy_kmap(kmap_t *kmap, kbinding_t *trie);
static int _editor_add_macro_by_str(editor_t *editor, char *str);
static void _editor_init_syntaxes(editor_t *editor);
static void _editor_init_syntax(editor_t *editor, syntax_t **optret_syntax, char *name, char *path_pattern, int tab_width, int tab_to_space, srule_def_t *defs);
static int _editor_init_syntax_by_str(editor_t *editor, syntax_t **ret_syntax, char *str);
static void _editor_init_syntax_add_rule(syntax_t *syntax, srule_def_t *def);
static int _editor_init_syntax_add_rule_by_str(syntax_t *syntax, char *str);
static void _editor_destroy_syntax_map(syntax_t *map);
static int _editor_init_from_rc_read(editor_t *editor, FILE *rc, char **ret_rc_data, size_t *ret_rc_data_len);
static int _editor_init_from_rc_exec(editor_t *editor, char *rc_path, char **ret_rc_data, size_t *ret_rc_data_len);
static int _editor_init_from_rc(editor_t *editor, FILE *rc, char *rc_path);
static int _editor_init_from_args(editor_t *editor, int argc, char **argv);
static void _editor_init_status(editor_t *editor);
static void _editor_init_bviews(editor_t *editor, int argc, char **argv);
static int _editor_init_headless_mode(editor_t *editor);
static int _editor_init_startup_macro(editor_t *editor);

// Init editor from args
int editor_init(editor_t *editor, int argc, char **argv) {
    int rv;
    FILE *rc;
    char *home_rc;
    rv = MLE_OK;
    do {
        // Create a resuable PCRE2 match data block. This is hacky / lazy. PCRE
        // is used all over the place, even in places where there's no easy way
        // to get at a shared state, e.g., mark.c. Also feels wasteful to keep
        // reallocating this thing, so let's just create one with 10 match
        // slots which is the most we ever use. Free in editor_deinit.
        pcre2_md = pcre2_match_data_create(10, NULL);

        // Set editor defaults
        editor->is_in_init = 1;
        editor->tab_width = MLE_DEFAULT_TAB_WIDTH;
        editor->tab_to_space = MLE_DEFAULT_TAB_TO_SPACE;
        editor->trim_paste = MLE_DEFAULT_TRIM_PASTE;
        editor->auto_indent = MLE_DEFAULT_AUTO_INDENT;
        editor->highlight_bracket_pairs = MLE_DEFAULT_HILI_BRACKET_PAIRS;
        editor->read_rc_file = MLE_DEFAULT_READ_RC_FILE;
        editor->soft_wrap = MLE_DEFAULT_SOFT_WRAP;
        editor->coarse_undo = MLE_DEFAULT_COARSE_UNDO;
        editor->viewport_scope_x = -4;
        editor->viewport_scope_y = -1;
        editor->color_col = -1;
        editor->exit_code = EXIT_SUCCESS;
        editor->headless_mode = isatty(STDIN_FILENO) == 0 ? 1 : 0;
        _editor_set_macro_toggle_key(editor, MLE_DEFAULT_MACRO_TOGGLE_KEY);

        // Init signal handlers
        _editor_init_signal_handlers(editor);

        // Register commands
        _editor_register_cmds(editor);

        // Init kmaps
        _editor_init_kmaps(editor);

        // Init syntaxes
        _editor_init_syntaxes(editor);

        // Parse rc files
        if (!_editor_should_skip_rc(argv)) {
            home_rc = NULL;
            if (getenv("HOME")) {
                asprintf(&home_rc, "%s/%s", getenv("HOME"), ".mlerc");
                if (util_is_file(home_rc, "rb", &rc)) {
                    rv = _editor_init_from_rc(editor, rc, home_rc);
                    fclose(rc);
                }
                free(home_rc);
            }
            if (rv != MLE_OK) break;
            if (util_is_file("/etc/mlerc", "rb", &rc)) {
                rv = _editor_init_from_rc(editor, rc, "/etc/mlerc");
                fclose(rc);
            }
            if (rv != MLE_OK) break;
        }

        // Parse cli args
        rv = _editor_init_from_args(editor, argc, argv);
        if (rv != MLE_OK) break;

        // Init status bar
        _editor_init_status(editor);

        // Init bviews
        _editor_init_bviews(editor, argc, argv);

        // Init startup macro
        _editor_init_headless_mode(editor);

        // Init headless mode
        _editor_init_startup_macro(editor);
    } while(0);

    editor->is_in_init = 0;
    return rv;
}

// Run editor
int editor_run(editor_t *editor) {
    loop_context_t loop_ctx;
    memset(&loop_ctx, 0, sizeof(loop_context_t));
    _editor_resize(editor, -1, -1);
    _editor_loop(editor, &loop_ctx);
    return MLE_OK;
}

// Deinit editor
int editor_deinit(editor_t *editor) {
    bview_t *bview;
    bview_t *bview_tmp1;
    bview_t *bview_tmp2;
    kmap_t *kmap;
    kmap_t *kmap_tmp;
    kmacro_t *macro;
    kmacro_t *macro_tmp;
    cmd_t *cmd;
    cmd_t *cmd_tmp;
    prompt_history_t *prompt_history;
    prompt_history_t *prompt_history_tmp;
    prompt_hnode_t *prompt_hnode;
    prompt_hnode_t *prompt_hnode_tmp1;
    prompt_hnode_t *prompt_hnode_tmp2;
    observer_t *observer;
    observer_t *observer_tmp;
    if (editor->status) bview_destroy(editor->status);
    CDL_FOREACH_SAFE2(editor->all_bviews, bview, bview_tmp1, bview_tmp2, all_prev, all_next) {
        CDL_DELETE2(editor->all_bviews, bview, all_prev, all_next);
        bview_destroy(bview);
    }
    HASH_ITER(hh, editor->kmap_map, kmap, kmap_tmp) {
        HASH_DEL(editor->kmap_map, kmap);
        _editor_destroy_kmap(kmap, kmap->bindings->children);
        if (kmap->default_cmd_name) free(kmap->default_cmd_name);
        free(kmap->bindings);
        free(kmap->name);
        free(kmap);
    }
    HASH_ITER(hh, editor->macro_map, macro, macro_tmp) {
        HASH_DEL(editor->macro_map, macro);
        if (macro->inputs) free(macro->inputs);
        if (macro->name) free(macro->name);
        free(macro);
    }
    HASH_ITER(hh, editor->cmd_map, cmd, cmd_tmp) {
        HASH_DEL(editor->cmd_map, cmd);
        _editor_destroy_cmd(editor, cmd);
    }
    HASH_ITER(hh, editor->prompt_history, prompt_history, prompt_history_tmp) {
        HASH_DEL(editor->prompt_history, prompt_history);
        free(prompt_history->prompt_str);
        CDL_FOREACH_SAFE(prompt_history->prompt_hlist, prompt_hnode, prompt_hnode_tmp1, prompt_hnode_tmp2) {
            CDL_DELETE(prompt_history->prompt_hlist, prompt_hnode);
            free(prompt_hnode->data);
            free(prompt_hnode);
        }
        free(prompt_history);
    }
    DL_FOREACH_SAFE(editor->observers, observer, observer_tmp) {
        editor_destroy_observer(editor, observer);
    }
    if (editor->macro_record) {
        if (editor->macro_record->inputs) free(editor->macro_record->inputs);
        free(editor->macro_record);
    }
    _editor_destroy_syntax_map(editor->syntax_map);
    if (editor->kmap_init_name) free(editor->kmap_init_name);
    if (editor->insertbuf) free(editor->insertbuf);
    if (editor->cut_buffer) free(editor->cut_buffer);
    if (editor->ttyfd) close(editor->ttyfd);
    if (editor->startup_macro_name) free(editor->startup_macro_name);

    pcre2_match_data_free(pcre2_md);

    return MLE_OK;
}

// Prompt user for input
int editor_prompt(editor_t *editor, char *prompt, editor_prompt_params_t *params, char **optret_answer) {
    bview_t *bview_tmp;
    loop_context_t loop_ctx;
    memset(&loop_ctx, 0, sizeof(loop_context_t));

    // Disallow nested prompts
    if (editor->prompt) {
        if (optret_answer) *optret_answer = NULL;
        return MLE_ERR;
    }

    // Init loop_ctx
    loop_ctx.invoker = editor->active;
    loop_ctx.should_exit = 0;
    loop_ctx.prompt_answer = NULL;

    // Init prompt
    editor_open_bview(editor, NULL, MLE_BVIEW_TYPE_PROMPT, NULL, 0, 1, 0, 0, NULL, &editor->prompt);
    if (params && params->prompt_cb) bview_add_listener(editor->prompt, params->prompt_cb, params->prompt_cb_udata);
    editor->prompt->prompt_str = prompt;
    bview_push_kmap(editor->prompt, params && params->kmap ? params->kmap : editor->kmap_prompt_input);

    // Insert data if present
    if (params && params->data && params->data_len > 0) {
        buffer_insert(editor->prompt->buffer, 0, params->data, params->data_len, NULL);
        mark_move_eol(editor->prompt->active_cursor->mark);
    }

    // Loop inside prompt
    _editor_loop(editor, &loop_ctx);

    // Set answer
    if (optret_answer) {
        *optret_answer = loop_ctx.prompt_answer;
    } else if (loop_ctx.prompt_answer) {
        free(loop_ctx.prompt_answer);
        loop_ctx.prompt_answer = NULL;
    }

    // Restore previous focus
    bview_tmp = editor->prompt;
    editor->prompt = NULL;
    editor_close_bview(editor, bview_tmp, NULL);
    editor_set_active(editor, loop_ctx.invoker);

    return MLE_OK;
}

// Open dialog menu
int editor_menu(editor_t *editor, cmd_func_t callback, char *opt_buf_data, int opt_buf_data_len, aproc_t *opt_aproc, bview_t **optret_menu) {
    bview_t *menu;
    editor_open_bview(editor, NULL, MLE_BVIEW_TYPE_EDIT, NULL, 0, 1, 0, 0, NULL, &menu);
    menu->is_menu = 1;
    menu->soft_wrap = 0;
    menu->menu_callback = callback;
    bview_push_kmap(menu, editor->kmap_menu);
    if (opt_aproc) {
        aproc_set_owner(opt_aproc, menu, &(menu->aproc));
    }
    if (opt_buf_data) {
        mark_insert_before(menu->active_cursor->mark, opt_buf_data, opt_buf_data_len);
    }
    if (optret_menu) *optret_menu = menu;
    return MLE_OK;
}

// Open a bview
int editor_open_bview(editor_t *editor, bview_t *opt_parent, int type, char *opt_path, int opt_path_len, int make_active, bint_t linenum, int skip_resize, buffer_t *opt_buffer, bview_t **optret_bview) {
    bview_t *bview;
    bview_rect_t *rect;
    int found;
    found = 0;
    // Check if already open and not dirty
    if (opt_path) {
        CDL_FOREACH2(editor->all_bviews, bview, all_next) {
            if (bview->buffer
                && !bview->buffer->is_unsaved
                && bview->buffer->path
                && strlen(bview->buffer->path) == (size_t)opt_path_len
                && strncmp(opt_path, bview->buffer->path, opt_path_len) == 0
            ) {
                found = 1;
                break;
            }
        }
    }
    // Make new bview if not already open
    if (!found) {
        bview = bview_new(editor, type, opt_path, opt_path_len, opt_buffer);
        CDL_APPEND2(editor->all_bviews, bview, all_prev, all_next);
        if (!opt_parent) {
            DL_APPEND2(editor->top_bviews, bview, top_prev, top_next);
        } else {
            opt_parent->split_child = bview;
        }
    }
    if (make_active) {
        editor_set_active(editor, bview);
    }
    if (!found && !editor->is_in_init && !skip_resize) {
        switch (type) {
            case MLE_BVIEW_TYPE_STATUS: rect = &editor->rect_status; break;
            case MLE_BVIEW_TYPE_PROMPT: rect = &editor->rect_prompt; break;
            default:
            case MLE_BVIEW_TYPE_EDIT:   rect = &editor->rect_edit;   break;
        }
        bview_resize(bview, rect->x, rect->y, rect->w, rect->h);
    }
    if (linenum > 0) {
        mark_move_to(bview->active_cursor->mark, linenum - 1, 0);
        bview_center_viewport_y(bview);
    }
    if (optret_bview) {
        *optret_bview = bview;
    }
    if (!found && opt_path && util_is_dir(opt_path)) {
        // TODO This is hacky
        cmd_context_t ctx;
        memset(&ctx, 0, sizeof(cmd_context_t));
        ctx.editor = editor;
        ctx.static_param = strndup(opt_path, opt_path_len);
        ctx.bview = bview;
        cmd_browse(&ctx);
        editor_close_bview(editor, bview, NULL);
        free(ctx.static_param);
        ctx.static_param = NULL;
    }
    return MLE_OK;
}

// Close a bview
int editor_close_bview(editor_t *editor, bview_t *bview, int *optret_num_closed) {
    int rc;
    if (optret_num_closed) *optret_num_closed = 0;
    if ((rc = _editor_close_bview_inner(editor, bview, optret_num_closed)) == MLE_OK) {
        _editor_resize(editor, editor->w, editor->h);
    }
    return rc;
}

// Set the active bview
int editor_set_active(editor_t *editor, bview_t *bview) {
    if (!_editor_bview_exists(editor, bview)) {
        MLE_RETURN_ERR(editor, "No bview %p in editor->all_bviews", (void*)bview);
    } else if (editor->prompt) {
        MLE_RETURN_ERR(editor, "Cannot abandon prompt for bview %p", (void*)bview);
    }
    editor->active = bview;
    if (MLE_BVIEW_IS_EDIT(bview)) {
        editor->active_edit = bview;
        editor->active_edit_root = bview_get_split_root(bview);
    }
    bview_rectify_viewport(bview);
    return MLE_OK;
}

// Print debug info
int editor_debug_dump(editor_t *editor, FILE *fp) {
    bview_t *bview;
    cursor_t *cursor;
    buffer_t *buffer;
    bline_t *bline;
    bint_t c;
    int bview_index;
    int cursor_index;
    bview_index = 0;
    CDL_FOREACH2(editor->all_bviews, bview, all_next) {
        if (!MLE_BVIEW_IS_EDIT(bview)) continue;
        cursor_index = 0;
        DL_FOREACH(bview->cursors, cursor) {
            fprintf(fp, "bview.%d.cursor.%d.mark.line_index=%" PRIdMAX "\n", bview_index, cursor_index, cursor->mark->bline->line_index);
            fprintf(fp, "bview.%d.cursor.%d.mark.col=%" PRIdMAX "\n", bview_index, cursor_index, cursor->mark->col);
            if (cursor->is_anchored) {
                fprintf(fp, "bview.%d.cursor.%d.anchor.line_index=%" PRIdMAX "\n", bview_index, cursor_index, cursor->anchor->bline->line_index);
                fprintf(fp, "bview.%d.cursor.%d.anchor.col=%" PRIdMAX "\n", bview_index, cursor_index, cursor->anchor->col);
            }
            fprintf(fp, "bview.%d.cursor.%d.sel_rule=%c\n", bview_index, cursor_index, cursor->sel_rule ? 'y' : 'n');
            cursor_index += 1;
        }
        fprintf(fp, "bview.%d.cursor_count=%d\n", bview_index, cursor_index);
        buffer = bview->buffer;
        fprintf(fp, "bview.%d.buffer.byte_count=%" PRIdMAX "\n", bview_index, buffer->byte_count);
        fprintf(fp, "bview.%d.buffer.line_count=%" PRIdMAX "\n", bview_index, buffer->line_count);
        fprintf(fp, "bview.%d.buffer.path=%s\n", bview_index, buffer->path ? buffer->path : "");
        for (bline = buffer->first_line; bline != NULL; bline = bline->next) {
            fprintf(fp, "bview.%d.buffer.blines.%" PRIdMAX ".chars=", bview_index, bline->line_index);
            for (c = 0; c < bline->char_count; ++c) {
                fprintf(fp, "<ch=%" PRIu32 " fg=%" PRIu16 " bg=%" PRIu16 ">",
                    bline->chars[c].ch,
                    bline->chars[c].style.fg,
                    bline->chars[c].style.bg
                );
            }
            fprintf(fp, "\n");
        }
        fprintf(fp, "bview.%d.buffer.blines_end\n", bview_index);
        fprintf(fp, "bview.%d.buffer.data_begin\n", bview_index);
        buffer_write_to_file(buffer, fp, NULL);
        fprintf(fp, "\nbview.%d.buffer.data_end\n", bview_index);
        bview_index += 1;
    }
    fprintf(fp, "bview_count=%d\n", bview_index);
    return MLE_OK;
}

// Set macro toggle key
static int _editor_set_macro_toggle_key(editor_t *editor, char *key) {
    return _editor_key_to_input(key, &editor->macro_toggle_key);
}

// Return 1 if bview exists in editor, else return 0
static int _editor_bview_exists(editor_t *editor, bview_t *bview) {
    bview_t *tmp;
    CDL_FOREACH2(editor->all_bviews, tmp, all_next) {
        if (tmp == bview) return 1;
    }
    return 0;
}

// Return number of EDIT bviews open
int editor_bview_edit_count(editor_t *editor) {
    int count;
    bview_t *bview;
    count = 0;
    CDL_FOREACH2(editor->all_bviews, bview, all_next) {
        if (MLE_BVIEW_IS_EDIT(bview)) count += 1;
    }
    return count;
}

// Return number of bviews displaying buffer
int editor_count_bviews_by_buffer(editor_t *editor, buffer_t *buffer) {
    int count;
    bview_t *bview;
    count = 0;
    CDL_FOREACH2(editor->all_bviews, bview, all_next) {
        if (bview->buffer == buffer) count += 1;
    }
    return count;
}

// Register a command
static int _editor_register_cmd_fn(editor_t *editor, char *name, int (*func)(cmd_context_t *ctx)) {
    cmd_t cmd = {0};
    cmd.name = name;
    cmd.func = func;
    return editor_register_cmd(editor, &cmd);
}

// Register a command (extended)
int editor_register_cmd(editor_t *editor, cmd_t *cmd) {
    cmd_t *existing_cmd;
    cmd_t *new_cmd;
    HASH_FIND_STR(editor->cmd_map, cmd->name, existing_cmd);
    if (existing_cmd) return MLE_ERR;
    new_cmd = calloc(1, sizeof(cmd_t));
    *new_cmd = *cmd;
    new_cmd->name = strdup(new_cmd->name);
    HASH_ADD_KEYPTR(hh, editor->cmd_map, new_cmd->name, strlen(new_cmd->name), new_cmd);
    return MLE_OK;
}

// Get input from either macro or user
int editor_get_input(editor_t *editor, loop_context_t *loop_ctx, cmd_context_t *ctx) {
    ctx->is_user_input = 0;
    if (editor->macro_apply
        && editor->macro_apply_input_index < editor->macro_apply->inputs_len
    ) {
        // Get input from macro
        MLE_KINPUT_COPY(ctx->input, editor->macro_apply->inputs[editor->macro_apply_input_index]);
        editor->macro_apply_input_index += 1;
    } else {
        // Get input from user
        if (editor->macro_apply) {
            // Clear macro if present
            editor->macro_apply = NULL;
            editor->macro_apply_input_index = 0;
        }
        if (editor->headless_mode) {
            // Bail if in headless mode
            loop_ctx->should_exit = 1;
            return MLE_ERR;
        } else {
            // Get input from user
            _editor_get_user_input(editor, ctx);
            ctx->is_user_input = 1;
        }
    }
    if (editor->is_recording_macro && editor->macro_record) {
        // Record macro input
        _editor_record_macro_input(editor->macro_record, &ctx->input);
    }
    return MLE_OK;
}

// Display the editor
int editor_display(editor_t *editor) {
    bview_t *bview;
    if (editor->headless_mode) return MLE_OK;
    tb_clear();
    bview_draw(editor->active_edit_root);
    bview_draw(editor->status);
    if (editor->prompt) bview_draw(editor->prompt);
    DL_FOREACH2(editor->top_bviews, bview, top_next) {
        _editor_draw_cursors(editor, bview);
    }
    tb_present();
    return MLE_OK;
}

// Register a cmd observer
int editor_register_observer(editor_t *editor, char *event_patt, void *udata, observer_func_t fn_callback, observer_t **optret_observer) {
    observer_t *observer;
    observer = calloc(1, sizeof(observer_t));
    observer->event_patt = strdup(event_patt);
    observer->callback = fn_callback;
    observer->udata = udata;
    DL_APPEND(editor->observers, observer);
    if (optret_observer) *optret_observer = observer;
    return MLE_OK;
}

// Register a cmd observer
int editor_destroy_observer(editor_t *editor, observer_t *observer) {
    DL_DELETE(editor->observers, observer);
    free(observer->event_patt);
    free(observer);
    return MLE_OK;
}

// Return 1 if we should skip reading rc files
static int _editor_should_skip_rc(char **argv) {
    int skip = 0;
    while (*argv) {
        if (strcmp("-h", *argv) == 0 || strcmp("-N", *argv) == 0) {
            skip = 1;
            break;
        }
        argv++;
    }
    return skip;
}

// Close a bview
static int _editor_close_bview_inner(editor_t *editor, bview_t *bview, int *optret_num_closed) {
    if (!_editor_bview_exists(editor, bview)) {
        MLE_RETURN_ERR(editor, "No bview %p in editor->all_bviews", (void*)bview);
    }
    if (bview->split_child) {
        _editor_close_bview_inner(editor, bview->split_child, optret_num_closed);
    }
    if (bview->split_parent) {
        bview->split_parent->split_child = NULL;
        editor_set_active(editor, bview->split_parent);
    } else if (bview == editor->active_edit) {
        if (bview->all_prev && bview->all_prev != bview && MLE_BVIEW_IS_EDIT(bview->all_prev)) {
            editor_set_active(editor, bview->all_prev);
        } else if (bview->all_next && bview->all_next != bview && MLE_BVIEW_IS_EDIT(bview->all_next)) {
            editor_set_active(editor, bview->all_next);
        } else {
            editor_open_bview(editor, NULL, MLE_BVIEW_TYPE_EDIT, NULL, 0, 1, 0, 0, NULL, NULL);
        }
    }
    if (!bview->split_parent) {
        DL_DELETE2(editor->top_bviews, bview, top_prev, top_next);
    }
    CDL_DELETE2(editor->all_bviews, bview, all_prev, all_next);
    bview_destroy(bview);
    if (optret_num_closed) *optret_num_closed += 1;
    return MLE_OK;
}

// Destroy a command
static int _editor_destroy_cmd(editor_t *editor, cmd_t *cmd) {
    (void)editor;
    free(cmd->name);
    free(cmd);
    return MLE_OK;
}

// Invoked when user hits enter in a prompt_input
static int _editor_prompt_input_submit(cmd_context_t *ctx) {
    bint_t answer_len;
    char *answer;
    buffer_get(ctx->bview->buffer, &answer, &answer_len);
    ctx->loop_ctx->prompt_answer = strndup(answer, answer_len);
    _editor_prompt_history_append(ctx, ctx->loop_ctx->prompt_answer);
    ctx->loop_ctx->should_exit = 1;
    return MLE_OK;
}

// Invoke when user hits tab in a prompt_input
static int _editor_prompt_input_complete(cmd_context_t *ctx) {
    loop_context_t *loop_ctx;
    char *cmd;
    char *cmd_arg;
    char *terms;
    size_t terms_len;
    int num_terms;
    char *term;
    int term_index;

    loop_ctx = ctx->loop_ctx;

    // Update tab_complete_term and tab_complete_index
    if (loop_ctx->last_cmd_ctx.cmd && loop_ctx->last_cmd_ctx.cmd->func == _editor_prompt_input_complete) {
        loop_ctx->tab_complete_index += 1;
    } else if (ctx->bview->buffer->first_line->data_len < MLE_LOOP_CTX_MAX_COMPLETE_TERM_SIZE) {
        snprintf(
            loop_ctx->tab_complete_term,
            MLE_LOOP_CTX_MAX_COMPLETE_TERM_SIZE,
            "%.*s",
            (int)ctx->bview->buffer->first_line->data_len,
            ctx->bview->buffer->first_line->data
        );
        loop_ctx->tab_complete_index = 0;
    } else {
        return MLE_OK;
    }

     // Assemble compgen command
    cmd_arg = util_escape_shell_arg(
        loop_ctx->tab_complete_term,
        strlen(loop_ctx->tab_complete_term)
    );
    asprintf(&cmd, "compgen -f %s 2>/dev/null | sort", cmd_arg);

    // Run compgen command
    terms = NULL;
    terms_len = 0;
    util_shell_exec(ctx->editor, cmd, 1, NULL, 0, 0, "bash", &terms, &terms_len, NULL);
    free(cmd);
    free(cmd_arg);

    // Get number of terms
    // TODO valgrind thinks there's an error here
    num_terms = 0;
    term = strchr(terms, '\n');
    while (term) {
        num_terms += 1;
        term = strchr(term + 1, '\n');
    }

    // Bail if no terms
    if (num_terms < 1) {
        free(terms);
        return MLE_OK;
    }

    // Determine term index
    term_index = loop_ctx->tab_complete_index % num_terms;

    // Set prompt input to term
    term = strtok(terms, "\n");
    while (term != NULL) {
        if (term_index == 0) {
            buffer_set(ctx->bview->buffer, term, strlen(term));
            mark_move_eol(ctx->cursor->mark);
            break;
        } else {
            term_index -= 1;
        }
        term = strtok(NULL, "\n");
    }

    free(terms);
    return MLE_OK;
}

// Find or add a prompt history entry for the current prompt
static prompt_history_t *_editor_prompt_find_or_add_history(cmd_context_t *ctx, prompt_hnode_t **optret_prompt_hnode) {
    prompt_history_t *prompt_history;
    HASH_FIND_STR(ctx->editor->prompt_history, ctx->bview->prompt_str, prompt_history);
    if (!prompt_history) {
        prompt_history = calloc(1, sizeof(prompt_history_t));
        prompt_history->prompt_str = strdup(ctx->bview->prompt_str);
        HASH_ADD_KEYPTR(hh, ctx->editor->prompt_history, prompt_history->prompt_str, strlen(prompt_history->prompt_str), prompt_history);
    }
    if (!ctx->loop_ctx->prompt_hnode) {
        ctx->loop_ctx->prompt_hnode = prompt_history->prompt_hlist
            ? prompt_history->prompt_hlist->prev
            : NULL;
    }
    if (optret_prompt_hnode) {
        *optret_prompt_hnode = ctx->loop_ctx->prompt_hnode;
    }
    return prompt_history;
}

// Prompt history up
static int _editor_prompt_history_up(cmd_context_t *ctx) {
    prompt_hnode_t *prompt_hnode;
    _editor_prompt_find_or_add_history(ctx, &prompt_hnode);
    if (prompt_hnode) {
        ctx->loop_ctx->prompt_hnode = prompt_hnode->prev;
        buffer_set(ctx->buffer, prompt_hnode->data, prompt_hnode->data_len);
    }
    return MLE_OK;
}

// Prompt history down
static int _editor_prompt_history_down(cmd_context_t *ctx) {
    prompt_hnode_t *prompt_hnode;
    _editor_prompt_find_or_add_history(ctx, &prompt_hnode);
    if (prompt_hnode) {
        ctx->loop_ctx->prompt_hnode = prompt_hnode->next;
        buffer_set(ctx->buffer, prompt_hnode->data, prompt_hnode->data_len);
    }
    return MLE_OK;
}

// Prompt history append
static int _editor_prompt_history_append(cmd_context_t *ctx, char *data) {
    prompt_history_t *prompt_history;
    prompt_hnode_t *prompt_hnode;
    prompt_history = _editor_prompt_find_or_add_history(ctx, NULL);
    prompt_hnode = calloc(1, sizeof(prompt_hnode_t));
    prompt_hnode->data = strdup(data);
    prompt_hnode->data_len = (bint_t)strlen(data);
    CDL_APPEND(prompt_history->prompt_hlist, prompt_hnode);
    return MLE_OK;
}

// Invoked when user hits a in a prompt_yna
static int _editor_prompt_yna_all(cmd_context_t *ctx) {
    ctx->loop_ctx->prompt_answer = MLE_PROMPT_ALL;
    ctx->loop_ctx->should_exit = 1;
    return MLE_OK;
}

// Invoked when user hits y in a prompt_yn(a)
static int _editor_prompt_yn_yes(cmd_context_t *ctx) {
    ctx->loop_ctx->prompt_answer = MLE_PROMPT_YES;
    ctx->loop_ctx->should_exit = 1;
    return MLE_OK;
}

// Invoked when user hits n in a prompt_yn(a)
static int _editor_prompt_yn_no(cmd_context_t *ctx) {
    ctx->loop_ctx->prompt_answer = MLE_PROMPT_NO;
    ctx->loop_ctx->should_exit = 1;
    return MLE_OK;
}

// Invoked when user cancels (Ctrl-C) a prompt_(input|yn), or hits any key in a prompt_ok
static int _editor_prompt_cancel(cmd_context_t *ctx) {
    ctx->loop_ctx->prompt_answer = NULL;
    ctx->loop_ctx->should_exit = 1;
    return MLE_OK;
}

// Invoked when user hits enter in a menu
static int _editor_menu_submit(cmd_context_t *ctx) {
    if (ctx->bview->menu_callback) return ctx->bview->menu_callback(ctx);
    return MLE_OK;
}

// Invoked when user hits C-c in a menu
static int _editor_menu_cancel(cmd_context_t *ctx) {
    if (ctx->bview->aproc) aproc_destroy(ctx->bview->aproc, 1);
    return MLE_OK;
}

// Invoked when user hits down in a prompt_isearch
static int _editor_prompt_isearch_next(cmd_context_t *ctx) {
    if (ctx->editor->active_edit->isearch_rule) {
        mark_move_next_cre_nudge(ctx->editor->active_edit->active_cursor->mark, ctx->editor->active_edit->isearch_rule->cre);
        bview_center_viewport_y(ctx->editor->active_edit);
    }
    return MLE_OK;
}

// Invoked when user hits up in a prompt_isearch
static int _editor_prompt_isearch_prev(cmd_context_t *ctx) {
    if (ctx->editor->active_edit->isearch_rule) {
        mark_move_prev_cre(ctx->editor->active_edit->active_cursor->mark, ctx->editor->active_edit->isearch_rule->cre);
        bview_center_viewport_y(ctx->editor->active_edit);
    }
    return MLE_OK;
}

// Invoked when user hits up in a prompt_isearch
static int _editor_prompt_isearch_viewport_up(cmd_context_t *ctx) {
    return bview_set_viewport_y(ctx->editor->active_edit, ctx->editor->active_edit->viewport_y - 5, 0);
}

// Invoked when user hits up in a prompt_isearch
static int _editor_prompt_isearch_viewport_down(cmd_context_t *ctx) {
    return bview_set_viewport_y(ctx->editor->active_edit, ctx->editor->active_edit->viewport_y + 5, 0);
}

// Drops a cursor on each isearch match
static int _editor_prompt_isearch_drop_cursors(cmd_context_t *ctx) {
    bview_t *bview;
    mark_t *mark;
    pcre2_code *cre;
    cursor_t *orig_cursor;
    cursor_t *last_cursor;
    bint_t nchars;
    bview = ctx->editor->active_edit;
    if (!bview->isearch_rule) return MLE_OK;
    orig_cursor = bview->active_cursor;
    mark = bview->active_cursor->mark;
    cre = bview->isearch_rule->cre;
    mark_move_beginning(mark);
    last_cursor = NULL;
    while (mark_move_next_cre_ex(mark, cre, NULL, NULL, &nchars) == MLBUF_OK) {
        bview_add_cursor(bview, mark->bline, mark->col, &last_cursor);
        mark_move_by(mark, MLE_MAX(1, nchars));
    }
    if (last_cursor) {
        mark_join(orig_cursor->mark, last_cursor->mark);
        bview_remove_cursor(bview, last_cursor);
    }
    bview->active_cursor = orig_cursor;
    bview_center_viewport_y(bview);
    ctx->loop_ctx->prompt_answer = NULL;
    ctx->loop_ctx->should_exit = 1;
    return MLE_OK;
}

// Run editor loop
static void _editor_loop(editor_t *editor, loop_context_t *loop_ctx) {
    cmd_t *cmd;
    cmd_context_t cmd_ctx;

    // Increment loop_depth
    editor->loop_depth += 1;

    // Init cmd_context
    memset(&cmd_ctx, 0, sizeof(cmd_context_t));
    cmd_ctx.editor = editor;
    cmd_ctx.loop_ctx = loop_ctx;

    // Loop until editor should exit
    while (!loop_ctx->should_exit) {
        // Set loop_ctx
        editor->loop_ctx = loop_ctx;

        // Display editor
        if (!editor->is_display_disabled) {
            editor_display(editor);
        }

        // Bail if debug_exit_after_startup set
        if (editor->debug_exit_after_startup) {
            break;
        }

        // Check for async io
        // aproc_drain_all will bail and return 0 if there's any tty data
        if (editor->aprocs && aproc_drain_all(editor->aprocs, &editor->ttyfd)) {
            continue;
        }

        // Get input
        if (editor_get_input(editor, loop_ctx, &cmd_ctx) == MLE_ERR) {
            break;
        }

        // Toggle macro?
        if (_editor_maybe_toggle_macro(editor, &cmd_ctx.input)) {
            continue;
        }

        if ((cmd = _editor_get_command(editor, &cmd_ctx, NULL)) != NULL) {
            // Found cmd in kmap trie, now execute
            cmd_ctx.cmd = cmd;

            if (cmd_ctx.is_user_input && cmd->func == cmd_insert_data) {
                // Ingest user paste
                _editor_ingest_paste(editor, &cmd_ctx);
            } else if (cmd->func == cmd_repeat && loop_ctx->last_cmd_ctx.cmd) {
                // Repeat last command
                memcpy(&cmd_ctx, &loop_ctx->last_cmd_ctx, sizeof(cmd_ctx));
            }

            // Notify cmd:*:before observers
            _editor_notify_cmd_observers(&cmd_ctx, 1);

            // Refresh cmd ctx if observers changed anything
            _editor_refresh_cmd_context(editor, &cmd_ctx);

            // Execute cmd
            cmd_ctx.cmd->func(&cmd_ctx);

            // Lift any temp anchors
            _editor_maybe_lift_temp_anchors(&cmd_ctx);

            // Notify cmd:*:after observers
            _editor_notify_cmd_observers(&cmd_ctx, 0);

            // Copy cmd_ctx to last_cmd_ctx
            memcpy(&loop_ctx->last_cmd_ctx, &cmd_ctx, sizeof(cmd_ctx));

            loop_ctx->binding_node = NULL;
            cmd_ctx.wildcard_params_len = 0;
            cmd_ctx.numeric_params_len = 0;
            cmd_ctx.pastebuf_len = 0;
        } else if (loop_ctx->need_more_input) {
            // Need more input to find
        } else {
            // Not found, bad command
            loop_ctx->binding_node = NULL;
        }
    }

    // Free stuff
    if (cmd_ctx.pastebuf) free(cmd_ctx.pastebuf);
    str_free(&loop_ctx->last_insert);
    if (loop_ctx->input_trail) free(loop_ctx->input_trail);
    loop_ctx->input_trail_len = 0;
    loop_ctx->input_trail_cap = 0;

    // Decrement loop_depth
    editor->loop_depth -= 1;
}

// Set fresh values on cmd_context
static void _editor_refresh_cmd_context(editor_t *editor, cmd_context_t *cmd_ctx) {
    cmd_ctx->cursor = editor->active->active_cursor;
    cmd_ctx->bview = cmd_ctx->cursor->bview;
    cmd_ctx->buffer = cmd_ctx->bview->buffer;
}

// Notify cmd observers
static void _editor_notify_cmd_observers(cmd_context_t *ctx, int is_before) {
    char *event_name;
    if (!ctx->editor->observers) return;
    asprintf(&event_name, "cmd:%s:%s", ctx->cmd->name, is_before ? "before" : "after");
    _editor_refresh_cmd_context(ctx->editor, ctx);
    editor_notify_observers(ctx->editor, event_name, (void*)ctx);
    free(event_name);
}

// Notify observers
int editor_notify_observers(editor_t *editor, char *event_name, void *event_data) {
    observer_t *observer;
    DL_FOREACH(editor->observers, observer) {
        if (fnmatch(observer->event_patt, event_name, 0) == 0) {
            (observer->callback)(event_name, event_data, observer->udata);
        }
    }
    return MLE_OK;
}

// Force a redraw of the screen
int editor_force_redraw(editor_t *editor) {
    int w;
    int h;
    int x;
    int y;
    (void)editor;
    if (tb_width() >= 0) tb_shutdown();
    tb_init();
    tb_select_input_mode(TB_INPUT_ALT);
    tb_set_cursor(-1, -1);
    w = tb_width();
    h = tb_height();
    for (x = 0; x < w; x++) {
        for (y = 0; y < h; y++) {
            // This used to use char 160 (non-breaking space) but that seems to
            // leave artifacts on my setup at least. I forget why I didn't use a
            // normal space to begin with.
            tb_change_cell(x, y, ' ', 0, 0);
        }
    }
    tb_present();
    return MLE_OK;
}

// If input == editor->macro_toggle_key, toggle macro mode and return 1. Else
// return 0.
static int _editor_maybe_toggle_macro(editor_t *editor, kinput_t *input) {
    char *name;
    if (memcmp(input, &editor->macro_toggle_key, sizeof(kinput_t)) != 0) {
        return 0;
    }
    if (editor->is_recording_macro) {
        // Stop recording macro and add to map
        if (editor->macro_record->inputs_len > 0) {
            // Remove toggle key from macro inputs
            editor->macro_record->inputs_len -= 1; // TODO This is hacky
        }
        HASH_ADD_STR(editor->macro_map, name, editor->macro_record);
        editor->macro_record = NULL;
        editor->is_recording_macro = 0;
    } else {
        // Get macro name and start recording
        editor_prompt(editor, "record_macro: Name?", NULL, &name);
        if (!name) return 1;
        editor->macro_record = calloc(1, sizeof(kmacro_t));
        editor->macro_record->name = name;
        editor->is_recording_macro = 1;
    }
    return 1;
}

// Resize the editor
static void _editor_resize(editor_t *editor, int w, int h) {
    bview_t *bview;
    bview_rect_t *bounds;

    editor->w = w >= 0 ? w : tb_width();
    editor->h = h >= 0 ? h : tb_height();

    editor->rect_edit.x = 0;
    editor->rect_edit.y = 0;
    editor->rect_edit.w = editor->w;
    editor->rect_edit.h = editor->h - 2;

    editor->rect_status.x = 0;
    editor->rect_status.y = editor->h - 2;
    editor->rect_status.w = editor->w;
    editor->rect_status.h = 1;

    editor->rect_prompt.x = 0;
    editor->rect_prompt.y = editor->h - 1;
    editor->rect_prompt.w = editor->w;
    editor->rect_prompt.h = 1;

    DL_FOREACH2(editor->top_bviews, bview, top_next) {
        if (MLE_BVIEW_IS_PROMPT(bview)) {
            bounds = &editor->rect_prompt;
        } else if (MLE_BVIEW_IS_STATUS(bview)) {
            bounds = &editor->rect_status;
        } else {
            if (bview->split_parent) continue;
            bounds = &editor->rect_edit;
        }
        bview_resize(bview, bounds->x, bounds->y, bounds->w, bounds->h);
    }
}

// Lift temporary anchors on cursor if not running cmd_move_temp_anchor
static void _editor_maybe_lift_temp_anchors(cmd_context_t *ctx) {
    cursor_t *cursor;
    if (ctx->cmd->func == cmd_move_temp_anchor) {
        return;
    }
    _editor_refresh_cmd_context(ctx->editor, ctx);
    if (!ctx->cursor || !ctx->cursor->bview || !ctx->cursor->bview->active_cursor) {
        return;
    }
    DL_FOREACH(ctx->cursor->bview->cursors, cursor) {
        if (!cursor->is_asleep && cursor->is_temp_anchored) {
            cursor_toggle_anchor(cursor, 1);
        }
    }
}

// Draw bviews cursors recursively
static void _editor_draw_cursors(editor_t *editor, bview_t *bview) {
    if (MLE_BVIEW_IS_EDIT(bview) && bview_get_split_root(bview) != editor->active_edit_root) {
        return;
    }
    bview_draw_cursor(bview, bview == editor->active ? 1 : 0);
    if (bview->split_child) {
        _editor_draw_cursors(editor, bview->split_child);
    }
}

// Get user input
static void _editor_get_user_input(editor_t *editor, cmd_context_t *ctx) {
    int rc;
    tb_event_t ev;

//fprintf(stderr, "in _editor_get_user_input\n");

    // Use pastebuf_leftover if present
    if (ctx->has_pastebuf_leftover) {
        MLE_KINPUT_COPY(ctx->input, ctx->pastebuf_leftover);
        ctx->has_pastebuf_leftover = 0;
        return;
    }

    // Poll for event
    while (1) {
        rc = tb_poll_event(&ev);
        if (rc != TB_OK) {
            continue; // Error
        } else if (ev.type == TB_EVENT_RESIZE) {
            // Resize
            _editor_resize(editor, ev.w, ev.h);
            editor_display(editor);
            continue;
        }
        MLE_KINPUT_SET(ctx->input, ev.mod, ev.ch, ev.key);
        editor->user_input_count += 1;
        break;
    }
}

// Ingest available input until non-cmd_insert_data
static void _editor_ingest_paste(editor_t *editor, cmd_context_t *ctx) {
    int rc;
    tb_event_t ev;
    kinput_t input;
    cmd_t *cmd;

    while (1) {
        // Peek event
        rc = tb_peek_event(&ev, 0);
        if (rc != TB_OK) {
            break; // Error
        } else if (ev.type == TB_EVENT_RESIZE) {
            // Resize
            _editor_resize(editor, ev.w, ev.h);
            editor_display(editor);
            break;
        }
        MLE_KINPUT_SET(input, ev.mod, ev.ch, ev.key);
        // TODO check for macro key
        cmd = _editor_get_command(editor, ctx, &input);
        if (cmd && cmd->func == cmd_insert_data) {
            // Insert data; keep ingesting
            _editor_append_pastebuf(editor, ctx, &input);
        } else {
            // Not insert data; set leftover and stop ingesting
            ctx->has_pastebuf_leftover = 1;
            ctx->pastebuf_leftover = input;
            break;
        }
    }
}

static void _editor_append_pastebuf(editor_t *editor, cmd_context_t *ctx, kinput_t *input) {
    // Expand pastebuf if needed
    if (ctx->pastebuf_len + 1 > ctx->pastebuf_size) {
        ctx->pastebuf_size += MLE_PASTEBUF_INCR;
        ctx->pastebuf = realloc(ctx->pastebuf, sizeof(kinput_t) * ctx->pastebuf_size);
    }
    // Append
    ctx->pastebuf[ctx->pastebuf_len++] = *input;
}

// Copy input into macro buffer
static void _editor_record_macro_input(kmacro_t *macro, kinput_t *input) {
    if (!macro->inputs) {
        macro->inputs = calloc(8, sizeof(kinput_t));
        macro->inputs_len = 0;
        macro->inputs_cap = 8;
    } else if (macro->inputs_len + 1 > macro->inputs_cap) {
        macro->inputs_cap = macro->inputs_len + 8;
        macro->inputs = realloc(macro->inputs, macro->inputs_cap * sizeof(kinput_t));
    }
    memcpy(macro->inputs + macro->inputs_len, input, sizeof(kinput_t));
    macro->inputs_len += 1;
}

// Return command for input
static cmd_t *_editor_get_command(editor_t *editor, cmd_context_t *ctx, kinput_t *opt_paste_input) {
    loop_context_t *loop_ctx;
    kinput_t *input_cur;
    kbinding_t *binding_cur, *binding_next;
    kmap_node_t *kmap_node;
    int is_paste, is_numeric;
    size_t i;
    cmd_t *rv;

    // Init some vars
    loop_ctx = ctx->loop_ctx;
    is_paste = opt_paste_input ? 1 : 0;
    kmap_node = editor->active->kmap_tail;
    binding_cur = loop_ctx->binding_node ? loop_ctx->binding_node : kmap_node->kmap->bindings;
    rv = NULL;

    // Reset some per-call state
    loop_ctx->need_more_input = 0;
    loop_ctx->binding_node = NULL;

    if (!is_paste) {
        // Append to input_trail
        loop_ctx->input_trail_len += 1;
        if (loop_ctx->input_trail_len > loop_ctx->input_trail_cap) {
            loop_ctx->input_trail_cap = loop_ctx->input_trail_len;
            loop_ctx->input_trail = realloc(loop_ctx->input_trail, sizeof(kinput_t) * loop_ctx->input_trail_cap);
        }
        MLE_KINPUT_COPY(loop_ctx->input_trail[loop_ctx->input_trail_len - 1], ctx->input);
    }

    // Look for key binding
    while (1) {
        if (is_paste) {
            input_cur = opt_paste_input;
        } else {
            if (loop_ctx->input_trail_idx >= loop_ctx->input_trail_len) {
                loop_ctx->need_more_input = 1;
                loop_ctx->binding_node = binding_cur;
                break;
            }
            input_cur = &loop_ctx->input_trail[loop_ctx->input_trail_idx];
        }
        binding_next = _editor_get_kbinding_node(binding_cur, input_cur, ctx, is_paste, &is_numeric);
        if (binding_next) {
            // Binding found
            if ((is_numeric || binding_next->children) && !is_paste) {
                // Need more input
                binding_cur = binding_next;
                loop_ctx->input_trail_idx += 1;
                continue;
            } else if (binding_next->is_leaf) {
                // Found leaf
                ctx->static_param = binding_next->static_param;
                rv = _editor_resolve_cmd(editor, &(binding_next->cmd), binding_next->cmd_name);
                break;
            }
        } else {
            // Binding not found
            if (kmap_node->kmap->default_cmd_name) {
                // Fallback to default cmd
                rv = _editor_resolve_cmd(editor, &(kmap_node->kmap->default_cmd), kmap_node->kmap->default_cmd_name);
                break;
            } else if (kmap_node->kmap->allow_fallthru && kmap_node != kmap_node->prev) {
                // Fallback to previous kmap on stack
                kmap_node = kmap_node->prev;
                binding_cur = kmap_node->kmap->bindings;
                loop_ctx->input_trail_idx = 0;
                continue;
            }
        }
        break;
    }

    if (rv && !is_paste) {
        if (rv->func == cmd_insert_data && loop_ctx->input_trail_idx != loop_ctx->input_trail_len - 1) {
            // Restore lost input_trail for cmd_insert_data
            MLE_KINPUT_COPY(ctx->input, loop_ctx->input_trail[loop_ctx->input_trail_idx]);
            for (i = loop_ctx->input_trail_idx + 1; i < loop_ctx->input_trail_len; i++) {
                _editor_append_pastebuf(editor, ctx, &loop_ctx->input_trail[i]);
            }
        }
        // Reset input_trail
        loop_ctx->input_trail_idx = 0;
        loop_ctx->input_trail_len = 0;
    }

    if (!rv && !loop_ctx->need_more_input) {
        // No chance of command being found
        loop_ctx->input_trail_idx = 0;
        loop_ctx->input_trail_len = 0;
    }

    return rv;
}

// Find binding by input in trie, taking into account numeric and wildcards patterns
static kbinding_t *_editor_get_kbinding_node(kbinding_t *node, kinput_t *input, cmd_context_t *ctx, int is_paste, int *ret_is_numeric) {
    kbinding_t *binding;
    kinput_t input_tmp;
    loop_context_t *loop_ctx;

    memset(&input_tmp, 0, sizeof(kinput_t));
    loop_ctx = ctx->loop_ctx;
    *ret_is_numeric = 0;

    if (!is_paste) {
        // Look for numeric .. TODO can be more efficient about this
        if (input->ch >= '0' && input->ch <= '9') {
            if (!loop_ctx->numeric_node) {
                MLE_KINPUT_SET_NUMERIC(input_tmp);
                HASH_FIND(hh, node->children, &input_tmp, sizeof(kinput_t), binding);
                loop_ctx->numeric_node = binding;
            }
            if (loop_ctx->numeric_node) {
                if (loop_ctx->numeric_len < MLE_LOOP_CTX_MAX_NUMERIC_LEN) {
                    loop_ctx->numeric[loop_ctx->numeric_len] = (char)input->ch;
                    loop_ctx->numeric_len += 1;
                    *ret_is_numeric = 1;
                    return node; // Need more input on this node
                }
                return NULL; // Ran out of `numeric` buffer .. TODO err
            }
        }

        // Parse/reset numeric buffer
        if (loop_ctx->numeric_len > 0) {
            if (ctx->numeric_params_len < MLE_MAX_NUMERIC_PARAMS) {
                loop_ctx->numeric[loop_ctx->numeric_len] = '\0';
                ctx->numeric_params[ctx->numeric_params_len] = strtoul(loop_ctx->numeric, NULL, 10);
                ctx->numeric_params_len += 1;
                loop_ctx->numeric_len = 0;
                node = loop_ctx->numeric_node; // Resume on numeric's children
                loop_ctx->numeric_node = NULL;
            } else {
                loop_ctx->numeric_len = 0;
                loop_ctx->numeric_node = NULL;
                return NULL; // Ran out of `numeric_params` space .. TODO err
            }
        }
    }

    // Look for input
    HASH_FIND(hh, node->children, input, sizeof(kinput_t), binding);
    if (binding) {
        return binding;
    }

    if (!is_paste) {
        // Look for wildcard
        MLE_KINPUT_SET_WILDCARD(input_tmp);
        HASH_FIND(hh, node->children, &input_tmp, sizeof(kinput_t), binding);
        if (binding) {
            if (ctx->wildcard_params_len < MLE_MAX_WILDCARD_PARAMS) {
                ctx->wildcard_params[ctx->wildcard_params_len] = input->ch;
                ctx->wildcard_params_len += 1;
            } else {
                return NULL; // Ran out of `wildcard_params` space .. TODO err
            }
            return binding;
        }
    }

    return NULL;
}

// Resolve a potentially unresolved cmd by name
static cmd_t *_editor_resolve_cmd(editor_t *editor, cmd_t **rcmd, char *cmd_name) {
    cmd_t *tcmd;
    cmd_t *cmd;
    cmd = NULL;
    if ((*rcmd)) {
        cmd = *rcmd;
    } else if (cmd_name) {
        HASH_FIND_STR(editor->cmd_map, cmd_name, tcmd);
        if (tcmd) {
            *rcmd = tcmd;
            cmd = tcmd;
        }
    }
    return cmd;
}

// Return a kinput_t given a key name
static int _editor_key_to_input(char *key, kinput_t *ret_input) {
    int keylen;
    uint32_t ch;
    char *dash, *c;

    keylen = strlen(key);
    memset(ret_input, 0, sizeof(kinput_t));

    // Check for CMS- modifier prefixes
    dash = keylen >= 2 ? strchr(key + 1, '-') : NULL;
    if (dash) {
        for (c = key; c < dash; c++) {
            switch (*c) {
                case 'C': ret_input->mod |= TB_MOD_CTRL; break;
                case 'M': ret_input->mod |= TB_MOD_ALT; break;
                case 'S': ret_input->mod |= TB_MOD_SHIFT; break;
            }
        }
        key = dash + 1;
        keylen = strlen(key);
    }

    // Check for special key names
    #define MLE_KEY_DEF(pkname, pmodmin, pmodadd, pch, pkey) \
        } else if (                                          \
            ((pmodmin) & ret_input->mod) == (pmodmin)        \
            && keylen == strlen((pkname))                    \
            && !strncmp((pkname), key, keylen)               \
        ) {                                                  \
            ret_input->ch = (pch);                           \
            ret_input->key = (pkey);                         \
            ret_input->mod |= (pmodadd);                     \
            return MLE_OK;
    if (keylen < 1) {
        return MLE_ERR;
        #include "keys.h"
    }
    #undef MLE_KEY_DEF

    // Check for Unicode code point
    ch = 0;
    if (utf8_char_to_unicode(&ch, key, NULL) != keylen || ch < 1) {
        return MLE_ERR;
    }
    ret_input->ch = ch;
    return MLE_OK;
}

// Init signal handlers
static void _editor_init_signal_handlers(editor_t *editor) {
    struct sigaction action;
    (void)editor;

    memset(&action, 0, sizeof(struct sigaction));

    action.sa_handler = _editor_graceful_exit;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGQUIT, &action, NULL);
    sigaction(SIGHUP, &action, NULL);

    action.sa_handler = _editor_continue;
    sigaction(SIGCONT, &action, NULL);

    signal(SIGPIPE, SIG_IGN);
}

// Resume editor after cmd_suspend
static void _editor_continue(int signum) {
    editor_force_redraw(&_editor);
}

// Gracefully exit
static void _editor_graceful_exit(int signum) {
    bview_t *bview;
    char path[64];
    int bview_num;
    (void)signum;
    bview_num = 0;
    if (tb_width() >= 0) tb_shutdown();
    CDL_FOREACH2(_editor.all_bviews, bview, all_next) {
        if (bview->buffer->is_unsaved) {
            snprintf((char*)&path, 64, ".mle.bak.%d.%d", getpid(), bview_num);
            buffer_save_as(bview->buffer, path, NULL);
            bview_num += 1;
        }
    }
    editor_deinit(&_editor);
    exit(1);
}

// Register built-in commands
static void _editor_register_cmds(editor_t *editor) {
    _editor_register_cmd_fn(editor, "cmd_align_cursors", cmd_align_cursors);
    _editor_register_cmd_fn(editor, "cmd_anchor_by", cmd_anchor_by);
    _editor_register_cmd_fn(editor, "cmd_apply_macro_by", cmd_apply_macro_by);
    _editor_register_cmd_fn(editor, "cmd_apply_macro", cmd_apply_macro);
    _editor_register_cmd_fn(editor, "cmd_blist", cmd_blist);
    _editor_register_cmd_fn(editor, "cmd_browse", cmd_browse);
    _editor_register_cmd_fn(editor, "cmd_close", cmd_close);
    _editor_register_cmd_fn(editor, "cmd_copy_by", cmd_copy_by);
    _editor_register_cmd_fn(editor, "cmd_copy", cmd_copy);
    _editor_register_cmd_fn(editor, "cmd_ctag", cmd_ctag);
    _editor_register_cmd_fn(editor, "cmd_cut_by", cmd_cut_by);
    _editor_register_cmd_fn(editor, "cmd_cut", cmd_cut);
    _editor_register_cmd_fn(editor, "cmd_delete_after", cmd_delete_after);
    _editor_register_cmd_fn(editor, "cmd_delete_before", cmd_delete_before);
    _editor_register_cmd_fn(editor, "cmd_delete_word_after", cmd_delete_word_after);
    _editor_register_cmd_fn(editor, "cmd_delete_word_before", cmd_delete_word_before);
    _editor_register_cmd_fn(editor, "cmd_drop_cursor_column", cmd_drop_cursor_column);
    _editor_register_cmd_fn(editor, "cmd_drop_sleeping_cursor", cmd_drop_sleeping_cursor);
    _editor_register_cmd_fn(editor, "cmd_find_word", cmd_find_word);
    _editor_register_cmd_fn(editor, "cmd_fsearch", cmd_fsearch);
    _editor_register_cmd_fn(editor, "cmd_fsearch_fzy", cmd_fsearch_fzy);
    _editor_register_cmd_fn(editor, "cmd_grep", cmd_grep);
    _editor_register_cmd_fn(editor, "cmd_indent", cmd_indent);
    _editor_register_cmd_fn(editor, "cmd_insert_data", cmd_insert_data);
    _editor_register_cmd_fn(editor, "cmd_insert_newline_above", cmd_insert_newline_above);
    _editor_register_cmd_fn(editor, "cmd_insert_newline_below", cmd_insert_newline_below);
    _editor_register_cmd_fn(editor, "cmd_isearch", cmd_isearch);
    _editor_register_cmd_fn(editor, "cmd_jump", cmd_jump);
    _editor_register_cmd_fn(editor, "cmd_less", cmd_less);
    _editor_register_cmd_fn(editor, "cmd_move_beginning", cmd_move_beginning);
    _editor_register_cmd_fn(editor, "cmd_move_bol", cmd_move_bol);
    _editor_register_cmd_fn(editor, "cmd_move_bracket_back", cmd_move_bracket_back);
    _editor_register_cmd_fn(editor, "cmd_move_bracket_forward", cmd_move_bracket_forward);
    _editor_register_cmd_fn(editor, "cmd_move_bracket_toggle", cmd_move_bracket_toggle);
    _editor_register_cmd_fn(editor, "cmd_move_down", cmd_move_down);
    _editor_register_cmd_fn(editor, "cmd_move_end", cmd_move_end);
    _editor_register_cmd_fn(editor, "cmd_move_eol", cmd_move_eol);
    _editor_register_cmd_fn(editor, "cmd_move_left", cmd_move_left);
    _editor_register_cmd_fn(editor, "cmd_move_page_down", cmd_move_page_down);
    _editor_register_cmd_fn(editor, "cmd_move_page_up", cmd_move_page_up);
    _editor_register_cmd_fn(editor, "cmd_move_relative", cmd_move_relative);
    _editor_register_cmd_fn(editor, "cmd_move_right", cmd_move_right);
    _editor_register_cmd_fn(editor, "cmd_move_temp_anchor", cmd_move_temp_anchor);
    _editor_register_cmd_fn(editor, "cmd_move_to_line", cmd_move_to_line);
    _editor_register_cmd_fn(editor, "cmd_move_to_offset", cmd_move_to_offset);
    _editor_register_cmd_fn(editor, "cmd_move_until_back", cmd_move_until_back);
    _editor_register_cmd_fn(editor, "cmd_move_until_forward", cmd_move_until_forward);
    _editor_register_cmd_fn(editor, "cmd_move_up", cmd_move_up);
    _editor_register_cmd_fn(editor, "cmd_move_word_back", cmd_move_word_back);
    _editor_register_cmd_fn(editor, "cmd_move_word_forward", cmd_move_word_forward);
    _editor_register_cmd_fn(editor, "cmd_next", cmd_next);
    _editor_register_cmd_fn(editor, "cmd_open_file", cmd_open_file);
    _editor_register_cmd_fn(editor, "cmd_open_new", cmd_open_new);
    _editor_register_cmd_fn(editor, "cmd_open_replace_file", cmd_open_replace_file);
    _editor_register_cmd_fn(editor, "cmd_open_replace_new", cmd_open_replace_new);
    _editor_register_cmd_fn(editor, "cmd_outdent", cmd_outdent);
    _editor_register_cmd_fn(editor, "cmd_perl", cmd_perl);
    _editor_register_cmd_fn(editor, "cmd_pop_kmap", cmd_pop_kmap);
    _editor_register_cmd_fn(editor, "cmd_prev", cmd_prev);
    _editor_register_cmd_fn(editor, "cmd_push_kmap", cmd_push_kmap);
    _editor_register_cmd_fn(editor, "cmd_quit", cmd_quit);
    _editor_register_cmd_fn(editor, "cmd_quit_without_saving", cmd_quit_without_saving);
    _editor_register_cmd_fn(editor, "cmd_redo", cmd_redo);
    _editor_register_cmd_fn(editor, "cmd_redraw", cmd_redraw);
    _editor_register_cmd_fn(editor, "cmd_remove_extra_cursors", cmd_remove_extra_cursors);
    _editor_register_cmd_fn(editor, "cmd_repeat", cmd_repeat);
    _editor_register_cmd_fn(editor, "cmd_replace", cmd_replace);
    _editor_register_cmd_fn(editor, "cmd_rfind_word", cmd_rfind_word);
    _editor_register_cmd_fn(editor, "cmd_rsearch", cmd_rsearch);
    _editor_register_cmd_fn(editor, "cmd_save_as", cmd_save_as);
    _editor_register_cmd_fn(editor, "cmd_save", cmd_save);
    _editor_register_cmd_fn(editor, "cmd_search", cmd_search);
    _editor_register_cmd_fn(editor, "cmd_search_next", cmd_search_next);
    _editor_register_cmd_fn(editor, "cmd_search_prev", cmd_search_prev);
    _editor_register_cmd_fn(editor, "cmd_set_opt", cmd_set_opt);
    _editor_register_cmd_fn(editor, "cmd_shell", cmd_shell);
    _editor_register_cmd_fn(editor, "cmd_show_help", cmd_show_help);
    _editor_register_cmd_fn(editor, "cmd_split_horizontal", cmd_split_horizontal);
    _editor_register_cmd_fn(editor, "cmd_split_vertical", cmd_split_vertical);
    _editor_register_cmd_fn(editor, "cmd_suspend", cmd_suspend);
    _editor_register_cmd_fn(editor, "cmd_swap_anchor", cmd_swap_anchor);
    _editor_register_cmd_fn(editor, "cmd_toggle_anchor", cmd_toggle_anchor);
    _editor_register_cmd_fn(editor, "cmd_uncut", cmd_uncut);
    _editor_register_cmd_fn(editor, "cmd_uncut_last", cmd_uncut_last);
    _editor_register_cmd_fn(editor, "cmd_undo", cmd_undo);
    _editor_register_cmd_fn(editor, "cmd_viewport_bot", cmd_viewport_bot);
    _editor_register_cmd_fn(editor, "cmd_viewport_mid", cmd_viewport_mid);
    _editor_register_cmd_fn(editor, "cmd_viewport_toggle", cmd_viewport_toggle);
    _editor_register_cmd_fn(editor, "cmd_viewport_top", cmd_viewport_top);
    _editor_register_cmd_fn(editor, "cmd_wake_sleeping_cursors", cmd_wake_sleeping_cursors);
    _editor_register_cmd_fn(editor, "_editor_menu_cancel", _editor_menu_cancel);
    _editor_register_cmd_fn(editor, "_editor_menu_submit", _editor_menu_submit);
    _editor_register_cmd_fn(editor, "_editor_prompt_cancel", _editor_prompt_cancel);
    _editor_register_cmd_fn(editor, "_editor_prompt_history_down", _editor_prompt_history_down);
    _editor_register_cmd_fn(editor, "_editor_prompt_history_up", _editor_prompt_history_up);
    _editor_register_cmd_fn(editor, "_editor_prompt_input_complete", _editor_prompt_input_complete);
    _editor_register_cmd_fn(editor, "_editor_prompt_input_submit", _editor_prompt_input_submit);
    _editor_register_cmd_fn(editor, "_editor_prompt_isearch_drop_cursors", _editor_prompt_isearch_drop_cursors);
    _editor_register_cmd_fn(editor, "_editor_prompt_isearch_next", _editor_prompt_isearch_next);
    _editor_register_cmd_fn(editor, "_editor_prompt_isearch_prev", _editor_prompt_isearch_prev);
    _editor_register_cmd_fn(editor, "_editor_prompt_isearch_viewport_down", _editor_prompt_isearch_viewport_down);
    _editor_register_cmd_fn(editor, "_editor_prompt_isearch_viewport_up", _editor_prompt_isearch_viewport_up);
    _editor_register_cmd_fn(editor, "_editor_prompt_yna_all", _editor_prompt_yna_all);
    _editor_register_cmd_fn(editor, "_editor_prompt_yn_no", _editor_prompt_yn_no);
    _editor_register_cmd_fn(editor, "_editor_prompt_yn_yes", _editor_prompt_yn_yes);
}

// Init built-in kmaps
static void _editor_init_kmaps(editor_t *editor) {
    _editor_init_kmap(editor, &editor->kmap_normal, "mle_normal", "cmd_insert_data", 0, (kbinding_def_t[]){
        MLE_KBINDING_DEF("cmd_show_help", "f2"),
        MLE_KBINDING_DEF("cmd_delete_before", "backspace"),
        MLE_KBINDING_DEF("cmd_delete_before", "C-h"),
        MLE_KBINDING_DEF("cmd_delete_after", "delete"),
        MLE_KBINDING_DEF("cmd_insert_newline_above", "M-i"),
        MLE_KBINDING_DEF("cmd_insert_newline_below", "M-u"),
        MLE_KBINDING_DEF("cmd_move_bol", "C-a"),
        MLE_KBINDING_DEF("cmd_move_bol", "home"),
        MLE_KBINDING_DEF("cmd_move_eol", "C-e"),
        MLE_KBINDING_DEF("cmd_move_eol", "end"),
        MLE_KBINDING_DEF("cmd_move_beginning", "M-\\"),
        MLE_KBINDING_DEF("cmd_move_end", "M-/"),
        MLE_KBINDING_DEF("cmd_move_left", "left"),
        MLE_KBINDING_DEF("cmd_move_right", "right"),
        MLE_KBINDING_DEF("cmd_move_up", "up"),
        MLE_KBINDING_DEF("cmd_move_down", "down"),
        MLE_KBINDING_DEF("cmd_move_page_up", "pgup"),
        MLE_KBINDING_DEF("cmd_move_page_down", "pgdn"),
        MLE_KBINDING_DEF("cmd_move_to_line", "M-g"),
        MLE_KBINDING_DEF("cmd_move_to_offset", "M-G"),
        MLE_KBINDING_DEF_EX("cmd_move_relative", "M-y ## u", "up"),
        MLE_KBINDING_DEF_EX("cmd_move_relative", "M-y ## d", "down"),
        MLE_KBINDING_DEF("cmd_move_until_forward", "M-' **"),
        MLE_KBINDING_DEF("cmd_move_until_back", "M-; **"),
        MLE_KBINDING_DEF("cmd_move_word_forward", "M-f"),
        MLE_KBINDING_DEF("cmd_move_word_forward", "C-right"),
        MLE_KBINDING_DEF("cmd_move_word_back", "M-b"),
        MLE_KBINDING_DEF("cmd_move_word_back", "C-left"),
        MLE_KBINDING_DEF("cmd_move_bracket_forward", "M-right"),
        MLE_KBINDING_DEF("cmd_move_bracket_back", "M-left"),
        MLE_KBINDING_DEF("cmd_move_bracket_toggle", "M-="),
        MLE_KBINDING_DEF_EX("cmd_move_temp_anchor", "S-up", "up"),
        MLE_KBINDING_DEF_EX("cmd_move_temp_anchor", "S-down", "down"),
        MLE_KBINDING_DEF_EX("cmd_move_temp_anchor", "S-left", "left"),
        MLE_KBINDING_DEF_EX("cmd_move_temp_anchor", "S-right", "right"),
        MLE_KBINDING_DEF_EX("cmd_move_temp_anchor", "S-home", "bol"),
        MLE_KBINDING_DEF_EX("cmd_move_temp_anchor", "S-end", "eol"),
        MLE_KBINDING_DEF_EX("cmd_move_temp_anchor", "S-pgup", "page_up"),
        MLE_KBINDING_DEF_EX("cmd_move_temp_anchor", "S-pgdn", "page_down"),
        MLE_KBINDING_DEF_EX("cmd_move_temp_anchor", "CS-right", "word_forward"),
        MLE_KBINDING_DEF_EX("cmd_move_temp_anchor", "CS-left", "word_back"),
        MLE_KBINDING_DEF("cmd_search", "C-f"),
        MLE_KBINDING_DEF("cmd_rsearch", "CM-f"),
        MLE_KBINDING_DEF("cmd_search_next", "C-g"),
        MLE_KBINDING_DEF("cmd_search_prev", "CM-g"),
        MLE_KBINDING_DEF("cmd_find_word", "C-v"),
        MLE_KBINDING_DEF("cmd_rfind_word", "CM-v"),
        MLE_KBINDING_DEF("cmd_isearch", "C-r"),
        MLE_KBINDING_DEF("cmd_repeat", "f5"),
        MLE_KBINDING_DEF("cmd_replace", "C-t"),
        MLE_KBINDING_DEF("cmd_cut", "C-k"),
        MLE_KBINDING_DEF("cmd_copy", "M-k"),
        MLE_KBINDING_DEF("cmd_uncut", "C-u"),
        MLE_KBINDING_DEF("cmd_uncut_last", "CM-u"),
        MLE_KBINDING_DEF("cmd_redraw", "M-x l"),
        MLE_KBINDING_DEF("cmd_less", "M-l"),
        MLE_KBINDING_DEF("cmd_viewport_top", "M-9"),
        MLE_KBINDING_DEF("cmd_viewport_toggle", "C-l"),
        MLE_KBINDING_DEF("cmd_viewport_bot", "M-0"),
        MLE_KBINDING_DEF("cmd_push_kmap", "M-x p"),
        MLE_KBINDING_DEF("cmd_pop_kmap", "M-x P"),
        MLE_KBINDING_DEF_EX("cmd_copy_by", "C-c d", "bracket"),
        MLE_KBINDING_DEF_EX("cmd_copy_by", "C-c w", "word"),
        MLE_KBINDING_DEF_EX("cmd_copy_by", "C-c s", "word_back"),
        MLE_KBINDING_DEF_EX("cmd_copy_by", "C-c f", "word_forward"),
        MLE_KBINDING_DEF_EX("cmd_copy_by", "C-c a", "bol"),
        MLE_KBINDING_DEF_EX("cmd_copy_by", "C-c e", "eol"),
        MLE_KBINDING_DEF_EX("cmd_copy_by", "C-c c", "string"),
        MLE_KBINDING_DEF_EX("cmd_copy_by", "C-c q", "all"),
        MLE_KBINDING_DEF_EX("cmd_cut_by", "C-d d", "bracket"),
        MLE_KBINDING_DEF_EX("cmd_cut_by", "C-d w", "word"),
        MLE_KBINDING_DEF_EX("cmd_cut_by", "C-d s", "word_back"),
        MLE_KBINDING_DEF_EX("cmd_cut_by", "C-d f", "word_forward"),
        MLE_KBINDING_DEF_EX("cmd_cut_by", "C-d a", "bol"),
        MLE_KBINDING_DEF_EX("cmd_cut_by", "C-d e", "eol"),
        MLE_KBINDING_DEF_EX("cmd_cut_by", "C-d c", "string"),
        MLE_KBINDING_DEF_EX("cmd_cut_by", "C-d q", "all"),
        MLE_KBINDING_DEF_EX("cmd_anchor_by", "C-2 d", "bracket"),
        MLE_KBINDING_DEF_EX("cmd_anchor_by", "C-2 w", "word"),
        MLE_KBINDING_DEF_EX("cmd_anchor_by", "C-2 s", "word_back"),
        MLE_KBINDING_DEF_EX("cmd_anchor_by", "C-2 f", "word_forward"),
        MLE_KBINDING_DEF_EX("cmd_anchor_by", "C-2 a", "bol"),
        MLE_KBINDING_DEF_EX("cmd_anchor_by", "C-2 e", "eol"),
        MLE_KBINDING_DEF_EX("cmd_anchor_by", "C-2 c", "string"),
        MLE_KBINDING_DEF_EX("cmd_anchor_by", "C-2 q", "all"),
        MLE_KBINDING_DEF("cmd_delete_word_before", "C-w"),
        MLE_KBINDING_DEF("cmd_delete_word_after", "M-d"),
        MLE_KBINDING_DEF("cmd_delete_word_after", "C-delete"),
        MLE_KBINDING_DEF("cmd_toggle_anchor", "M-a"),
        MLE_KBINDING_DEF("cmd_drop_sleeping_cursor", "C-/ ."),
        MLE_KBINDING_DEF("cmd_wake_sleeping_cursors", "C-/ a"),
        MLE_KBINDING_DEF("cmd_remove_extra_cursors", "C-/ /"),
        MLE_KBINDING_DEF("cmd_drop_cursor_column", "C-/ '"),
        MLE_KBINDING_DEF("cmd_swap_anchor", "C-/ ;"),
        MLE_KBINDING_DEF("cmd_align_cursors", "C-/ l"),
        MLE_KBINDING_DEF("cmd_apply_macro", "M-z"),
        MLE_KBINDING_DEF("cmd_apply_macro_by", "M-m **"),
        MLE_KBINDING_DEF("cmd_next", "M-n"),
        MLE_KBINDING_DEF("cmd_prev", "M-p"),
        MLE_KBINDING_DEF("cmd_split_vertical", "M-v"),
        MLE_KBINDING_DEF("cmd_split_horizontal", "M-h"),
        MLE_KBINDING_DEF("cmd_grep", "M-q"),
        MLE_KBINDING_DEF("cmd_fsearch", "C-p"),
        MLE_KBINDING_DEF("cmd_browse", "C-b"),
        MLE_KBINDING_DEF("cmd_blist", "C-\\"),
        MLE_KBINDING_DEF("cmd_undo", "C-z"),
        MLE_KBINDING_DEF("cmd_redo", "C-y"),
        MLE_KBINDING_DEF("cmd_save", "C-s"),
        MLE_KBINDING_DEF("cmd_save_as", "M-s"),
        MLE_KBINDING_DEF_EX("cmd_set_opt", "M-o a", "tab_to_space"),
        MLE_KBINDING_DEF_EX("cmd_set_opt", "M-o t", "tab_width"),
        MLE_KBINDING_DEF_EX("cmd_set_opt", "M-o y", "syntax"),
        MLE_KBINDING_DEF_EX("cmd_set_opt", "M-o w", "soft_wrap"),
        MLE_KBINDING_DEF_EX("cmd_set_opt", "M-o u", "coarse_undo"),
        MLE_KBINDING_DEF("cmd_open_new", "C-n"),
        MLE_KBINDING_DEF("cmd_open_file", "C-o"),
        MLE_KBINDING_DEF("cmd_open_replace_new", "C-q n"),
        MLE_KBINDING_DEF("cmd_open_replace_file", "C-q o"),
        MLE_KBINDING_DEF_EX("cmd_fsearch", "C-q p", "replace"),
        MLE_KBINDING_DEF("cmd_indent", "M-."),
        MLE_KBINDING_DEF("cmd_outdent", "M-,"),
        MLE_KBINDING_DEF("cmd_outdent", "backtab"),
        MLE_KBINDING_DEF("cmd_ctag", "f3"),
        MLE_KBINDING_DEF("cmd_shell", "M-e"),
        MLE_KBINDING_DEF("cmd_perl", "M-w"),
        MLE_KBINDING_DEF("cmd_jump", "M-j"),
        MLE_KBINDING_DEF("cmd_close", "M-c"),
        MLE_KBINDING_DEF("cmd_suspend", "f4"),
        MLE_KBINDING_DEF("cmd_quit", "C-x"),
        MLE_KBINDING_DEF(NULL, NULL)
    });
    _editor_init_kmap(editor, &editor->kmap_prompt_input, "mle_prompt_input", NULL, 1, (kbinding_def_t[]){
        MLE_KBINDING_DEF("_editor_prompt_input_submit", "enter"),
        MLE_KBINDING_DEF("_editor_prompt_input_complete", "tab"),
        MLE_KBINDING_DEF("_editor_prompt_cancel", "C-c"),
        MLE_KBINDING_DEF("_editor_prompt_cancel", "C-x"),
        MLE_KBINDING_DEF("_editor_prompt_cancel", "M-c"),
        MLE_KBINDING_DEF("_editor_prompt_history_up", "up"),
        MLE_KBINDING_DEF("_editor_prompt_history_down", "down"),
        MLE_KBINDING_DEF(NULL, NULL)
    });
    _editor_init_kmap(editor, &editor->kmap_prompt_yn, "mle_prompt_yn", NULL, 0, (kbinding_def_t[]){
        MLE_KBINDING_DEF("_editor_prompt_yn_yes", "y"),
        MLE_KBINDING_DEF("_editor_prompt_yn_no", "n"),
        MLE_KBINDING_DEF("_editor_prompt_cancel", "C-c"),
        MLE_KBINDING_DEF("_editor_prompt_cancel", "C-x"),
        MLE_KBINDING_DEF("_editor_prompt_cancel", "M-c"),
        MLE_KBINDING_DEF(NULL, NULL)
    });
    _editor_init_kmap(editor, &editor->kmap_prompt_yna, "mle_prompt_yna", NULL, 0, (kbinding_def_t[]){
        MLE_KBINDING_DEF("_editor_prompt_yn_yes", "y"),
        MLE_KBINDING_DEF("_editor_prompt_yn_no", "n"),
        MLE_KBINDING_DEF("_editor_prompt_yna_all", "a"),
        MLE_KBINDING_DEF("_editor_prompt_cancel", "C-c"),
        MLE_KBINDING_DEF("_editor_prompt_cancel", "C-x"),
        MLE_KBINDING_DEF("_editor_prompt_cancel", "M-c"),
        MLE_KBINDING_DEF(NULL, NULL)
    });
    _editor_init_kmap(editor, &editor->kmap_prompt_ok, "mle_prompt_ok", "_editor_prompt_cancel", 0, (kbinding_def_t[]){
        MLE_KBINDING_DEF(NULL, NULL)
    });
    _editor_init_kmap(editor, &editor->kmap_menu, "mle_menu", NULL, 1, (kbinding_def_t[]){
        MLE_KBINDING_DEF("_editor_menu_submit", "enter"),
        MLE_KBINDING_DEF("_editor_menu_cancel", "C-c"),
        MLE_KBINDING_DEF(NULL, NULL)
    });
    _editor_init_kmap(editor, &editor->kmap_prompt_isearch, "mle_prompt_isearch", NULL, 1, (kbinding_def_t[]){
        MLE_KBINDING_DEF("_editor_prompt_isearch_prev", "up"),
        MLE_KBINDING_DEF("_editor_prompt_isearch_next", "down"),
        MLE_KBINDING_DEF("_editor_prompt_isearch_viewport_up", "pgup"),
        MLE_KBINDING_DEF("_editor_prompt_isearch_viewport_down", "pgdn"),
        MLE_KBINDING_DEF("_editor_prompt_isearch_drop_cursors", "C-/"),
        MLE_KBINDING_DEF("_editor_prompt_cancel", "enter"),
        MLE_KBINDING_DEF("_editor_prompt_cancel", "C-c"),
        MLE_KBINDING_DEF("_editor_prompt_cancel", "C-x"),
        MLE_KBINDING_DEF("_editor_prompt_cancel", "M-c"),
        MLE_KBINDING_DEF(NULL, NULL)
    });
}

// Init a single kmap
static void _editor_init_kmap(editor_t *editor, kmap_t **ret_kmap, char *name, char *default_cmd_name, int allow_fallthru, kbinding_def_t *defs) {
    kmap_t *kmap;

    kmap = calloc(1, sizeof(kmap_t));
    kmap->name = strdup(name);
    kmap->allow_fallthru = allow_fallthru;
    kmap->bindings = calloc(1, sizeof(kbinding_t));
    if (default_cmd_name) {
        kmap->default_cmd_name = strdup(default_cmd_name);
    }

    while (defs && defs->cmd_name) {
        _editor_init_kmap_add_binding(editor, kmap, defs);
        defs++;
    }

    HASH_ADD_KEYPTR(hh, editor->kmap_map, kmap->name, strlen(kmap->name), kmap);
    *ret_kmap = kmap;
}

// Add a binding to a kmap
static void _editor_init_kmap_add_binding(editor_t *editor, kmap_t *kmap, kbinding_def_t *binding_def) {
    char *cur_key_patt;
    cur_key_patt = strdup(binding_def->key_patt);
    _editor_init_kmap_add_binding_to_trie(&kmap->bindings->children, binding_def->cmd_name, cur_key_patt, binding_def->key_patt, binding_def->static_param);
    if (strcmp(binding_def->cmd_name, "cmd_show_help") == 0) {
        // TODO kind of hacky
        MLE_SET_INFO(editor, "show_help: Press %s", cur_key_patt);
    }
    free(cur_key_patt);
}

// Add a binding to a kmap trie
static int _editor_init_kmap_add_binding_to_trie(kbinding_t **trie, char *cmd_name, char *cur_key_patt, char *full_key_patt, char *static_param) {
    char *next_key_patt;
    kbinding_t *node;
    kinput_t input;

    // Find next_key_patt and add null-char to cur_key_patt
    next_key_patt = strchr(cur_key_patt, ' ');
    if (next_key_patt != NULL) {
        *next_key_patt = '\0';
        next_key_patt += 1;
    }
    // cur_key_patt points to a null-term cstring now
    // next_key_patt is either NULL or points to a null-term cstring

    // Parse cur_key_patt token as input
    memset(&input, 0, sizeof(kinput_t));
    if (strcmp("##", cur_key_patt) == 0) {
        MLE_KINPUT_SET_NUMERIC(input);
    } else if (strcmp("**", cur_key_patt) == 0) {
        MLE_KINPUT_SET_WILDCARD(input);
    } else if (_editor_key_to_input(cur_key_patt, &input) == MLE_OK) {
        // Hi mom!
    } else {
        return MLE_ERR;
    }

    // Add node for input if it doesn't already exist
    node = NULL;
    HASH_FIND(hh, *trie, &input, sizeof(kinput_t), node);
    if (!node) {
        node = calloc(1, sizeof(kbinding_t));
        MLE_KINPUT_COPY(node->input, input);
        HASH_ADD(hh, *trie, input, sizeof(kinput_t), node);
    }

    if (next_key_patt) {
        // Recurse for next key
        if (_editor_init_kmap_add_binding_to_trie(&node->children, cmd_name, next_key_patt, full_key_patt, static_param) != MLE_OK) {
            free(node);
            return MLE_ERR;
        }
    } else {
        // Leaf node, set cmd
        node->static_param = static_param ? strdup(static_param) : NULL;
        node->key_patt = strdup(full_key_patt);
        node->cmd_name = strdup(cmd_name);
        node->is_leaf = 1;
    }

    return MLE_OK;
}

// Proxy for _editor_init_kmap with str in format '<name>,<default_cmd>,<allow_fallthru>'
static int _editor_init_kmap_by_str(editor_t *editor, kmap_t **ret_kmap, char *str) {
    char *args[3];
    args[0] = strtok(str,  ","); if (!args[0]) return MLE_ERR;
    args[1] = strtok(NULL, ","); if (!args[1]) return MLE_ERR;
    args[2] = strtok(NULL, ",");
    _editor_init_kmap(editor, ret_kmap, args[0], args[2] ? args[1] : NULL, atoi(args[2] ? args[2] : args[1]), NULL);
    return MLE_OK;
}

// Proxy for _editor_init_kmap_add_binding with str in format '<cmd>,<key>,<param>'
static int _editor_init_kmap_add_binding_by_str(editor_t *editor, kmap_t *kmap, char *str) {
    char *args[3];
    args[0] = strtok(str,  ","); if (!args[0]) return MLE_ERR;
    args[1] = strtok(NULL, ","); if (!args[1]) return MLE_ERR;
    args[2] = strtok(NULL, ",");
    _editor_init_kmap_add_binding(editor, kmap, &((kbinding_def_t){args[0], args[1], args[2]}));
    return MLE_OK;
}

// Destroy a kmap
static void _editor_destroy_kmap(kmap_t *kmap, kbinding_t *trie) {
    kbinding_t *binding;
    kbinding_t *binding_tmp;
    int is_top;
    is_top = (trie == kmap->bindings ? 1 : 0);
    HASH_ITER(hh, trie, binding, binding_tmp) {
        if (binding->children) {
            _editor_destroy_kmap(kmap, binding->children);
        }
        HASH_DELETE(hh, trie, binding);
        if (binding->static_param) free(binding->static_param);
        if (binding->cmd_name) free(binding->cmd_name);
        if (binding->key_patt) free(binding->key_patt);
        free(binding);
    }
    if (is_top) {
        if (kmap->name) free(kmap->name);
        free(kmap);
    }
}

// Add a macro by str with format '<name> <key1> <key2> ... <keyN>'
static int _editor_add_macro_by_str(editor_t *editor, char *str) {
    int has_input;
    char *token;
    kmacro_t *macro;
    kinput_t input;

    has_input = 0;
    macro = NULL;

    // Tokenize by space
    token = strtok(str, " ");
    while (token) {
        if (!macro) {
            // Make macro with <name> on first token
            macro = calloc(1, sizeof(kmacro_t));
            macro->name = strdup(token);
        } else {
            // Parse token as kinput_t
            if (_editor_key_to_input(token, &input) != MLE_OK) {
                free(macro->name);
                free(macro);
                return MLE_ERR;
            }
            // Add kinput_t to macro
            _editor_record_macro_input(macro, &input);
            has_input = 1;
        }
        // Get next token
        token = strtok(NULL, " ");
    }

    // Add macro to map if has_input
    if (has_input) {
        HASH_ADD_KEYPTR(hh, editor->macro_map, macro->name, strlen(macro->name), macro);
        return MLE_OK;
    }

    // Fail
    if (macro) {
        free(macro->name);
        free(macro);
    }
    return MLE_ERR;
}

// Init built-in syntax map
static void _editor_init_syntaxes(editor_t *editor) {
    _editor_init_syntax(editor, NULL, "syn_generic", "\\.(c|cc|cpp|h|hh|hpp|php|py|rb|erb|sh|pl|go|js|java|jsp|lua|rs|zig)$", -1, -1, (srule_def_t[]){
        { "(?<![\\w%@$])("
          "abstract|alias|alignas|alignof|and|and_eq|arguments|array|as|asm|"
          "assert|auto|base|begin|bitand|bitor|bool|boolean|break|byte|"
          "callable|case|catch|chan|char|checked|class|clone|cmp|compl|const|"
          "const_cast|constexpr|continue|debugger|decimal|declare|decltype|"
          "def|default|defer|defined|del|delegate|delete|die|do|done|double|"
          "dynamic_cast|echo|elif|else|elseif|elsif|empty|end|enddeclare|"
          "endfor|endforeach|endif|endswitch|endwhile|ensure|enum|eq|esac|"
          "eval|event|except|exec|exit|exp|explicit|export|extends|extern|"
          "fallthrough|false|fi|final|finally|fixed|float|fn|for|foreach|"
          "friend|from|func|function|ge|global|go|goto|gt|if|implements|"
          "implicit|import|in|include|include_once|inline|instanceof|insteadof|"
          "int|interface|internal|is|isset|lambda|le|let|list|lock|long|lt|m|"
          "map|module|mutable|namespace|native|ne|new|next|nil|no|noexcept|not|"
          "not_eq|null|nullptr|object|operator|or|or_eq|out|override|package|"
          "params|pass|print|private|protected|public|q|qq|qr|qw|qx|raise|"
          "range|readonly|redo|ref|register|reinterpret_cast|require|"
          "require_once|rescue|retry|return|s|sbyte|sealed|select|self|short|"
          "signed|sizeof|stackalloc|static|static_assert|static_cast|"
          "strictfp|string|struct|sub|super|switch|synchronized|template|"
          "then|this|thread_local|throw|throws|time|tr|trait|transient|true|"
          "try|type|typedef|typeid|typename|typeof|uint|ulong|unchecked|"
          "undef|union|unless|unsafe|unset|unsigned|until|use|ushort|using|"
          "var|virtual|void|volatile|when|while|with|xor|xor_eq|y|yield"
          ")\\b", NULL, TB_GREEN, TB_DEFAULT },
        { "[(){}<>\\[\\].,;:?!+=/\\\\%^*-]", NULL, TB_RED | TB_BOLD, TB_DEFAULT },
        { "(?<!\\w)[\\%@$][a-zA-Z_$][a-zA-Z0-9_]*\\b", NULL, TB_GREEN, TB_DEFAULT },
        { "\\b[A-Z_][A-Z0-9_]*\\b", NULL, TB_RED | TB_BOLD, TB_DEFAULT },
        { "\\b(-?(0x)?[0-9]+|true|false|null)\\b", NULL, TB_BLUE | TB_BOLD, TB_DEFAULT },
        { "'([^']|\\')*?'", NULL, TB_YELLOW | TB_BOLD, TB_DEFAULT },
        { "\"(\\\"|[^\"])*?\"", NULL, TB_YELLOW | TB_BOLD, TB_DEFAULT },
        { "/" "/.*$", NULL, TB_CYAN, TB_DEFAULT },
        { "^\\s*#( .*|)$", NULL, TB_CYAN, TB_DEFAULT },
        { "^#!/.*$", NULL, TB_CYAN, TB_DEFAULT },
        { "/\\" "*", "\\*" "/", TB_CYAN, TB_DEFAULT },
        { "\\t+", NULL, TB_RED | TB_UNDERLINE, TB_DEFAULT },
        { "\\s+$", NULL, TB_GREEN | TB_UNDERLINE, TB_DEFAULT },
        { NULL, NULL, 0, 0 }
    });
}

// Init a single syntax
static void _editor_init_syntax(editor_t *editor, syntax_t **optret_syntax, char *name, char *path_pattern, int tab_width, int tab_to_space, srule_def_t *defs) {
    syntax_t *syntax;

    syntax = calloc(1, sizeof(syntax_t));
    syntax->name = strdup(name);
    syntax->path_pattern = strdup(path_pattern);
    syntax->tab_width = tab_width >= 1 ? tab_width : -1; // -1 means default
    syntax->tab_to_space = tab_to_space >= 0 ? (tab_to_space ? 1 : 0) : -1;

    while (defs && defs->re) {
        _editor_init_syntax_add_rule(syntax, defs);
        defs++;
    }
    HASH_ADD_KEYPTR(hh, editor->syntax_map, syntax->name, strlen(syntax->name), syntax);
    editor->syntax_last = syntax;

    if (optret_syntax) *optret_syntax = syntax;
}

// Proxy for _editor_init_syntax with str in format '<name>,<path_pattern>,<tab_width>,<tab_to_space>'
static int _editor_init_syntax_by_str(editor_t *editor, syntax_t **ret_syntax, char *str) {
    char *args[4];
    args[0] = strtok(str,  ","); if (!args[0]) return MLE_ERR;
    args[1] = strtok(NULL, ","); if (!args[1]) return MLE_ERR;
    args[2] = strtok(NULL, ","); if (!args[2]) return MLE_ERR;
    args[3] = strtok(NULL, ","); if (!args[3]) return MLE_ERR;
    _editor_init_syntax(editor, ret_syntax, args[0], args[1], atoi(args[2]), atoi(args[3]), NULL);
    return MLE_OK;
}

// Add rule to syntax
static void _editor_init_syntax_add_rule(syntax_t *syntax, srule_def_t *def) {
    srule_node_t *node;
    node = calloc(1, sizeof(srule_node_t));
    if (def->re_end) {
        node->srule = srule_new_multi(def->re, strlen(def->re), def->re_end, strlen(def->re_end), def->fg, def->bg);
    } else {
        node->srule = srule_new_single(def->re, strlen(def->re), 0, def->fg, def->bg);
    }
    if (node->srule) DL_APPEND(syntax->srules, node);
}

// Proxy for _editor_init_syntax_add_rule with str in format '<start>,<end>,<fg>,<bg>' or '<regex>,<fg>,<bg>'
static int _editor_init_syntax_add_rule_by_str(syntax_t *syntax, char *str) {
    char *args[4];
    int style_i;
    args[0] = strtok(str,  ","); if (!args[0]) return MLE_ERR;
    args[1] = strtok(NULL, ","); if (!args[1]) return MLE_ERR;
    args[2] = strtok(NULL, ","); if (!args[2]) return MLE_ERR;
    args[3] = strtok(NULL, ",");
    style_i = args[3] ? 2 : 1;
    _editor_init_syntax_add_rule(syntax, &((srule_def_t){ args[0], style_i == 2 ? args[1] : NULL, atoi(args[style_i]), atoi(args[style_i + 1]) }));
    return MLE_OK;
}

// Destroy a syntax
static void _editor_destroy_syntax_map(syntax_t *map) {
    syntax_t *syntax;
    syntax_t *syntax_tmp;
    srule_node_t *srule;
    srule_node_t *srule_tmp;
    HASH_ITER(hh, map, syntax, syntax_tmp) {
        HASH_DELETE(hh, map, syntax);
        DL_FOREACH_SAFE(syntax->srules, srule, srule_tmp) {
            DL_DELETE(syntax->srules, srule);
            srule_destroy(srule->srule);
            free(srule);
        }
        free(syntax->name);
        free(syntax->path_pattern);
        free(syntax);
    }
}

// Read rc file
static int _editor_init_from_rc_read(editor_t *editor, FILE *rc, char **ret_rc_data, size_t *ret_rc_data_len) {
    (void)editor;
    fseek(rc, 0L, SEEK_END);
    *ret_rc_data_len = (size_t)ftell(rc);
    fseek(rc, 0L, SEEK_SET);
    *ret_rc_data = malloc(*ret_rc_data_len + 1);
    fread(*ret_rc_data, *ret_rc_data_len, 1, rc);
    (*ret_rc_data)[*ret_rc_data_len] = '\0';
    return MLE_OK;
}

// Exec rc file, read stdout
static int _editor_init_from_rc_exec(editor_t *editor, char *rc_path, char **ret_rc_data, size_t *ret_rc_data_len) {
    char buf[512];
    size_t nbytes;
    char *data;
    size_t data_len;
    size_t data_cap;
    FILE *fp;

    // Popen rc file
    if ((fp = popen(rc_path, "r")) == NULL) {
        MLE_RETURN_ERR(editor, "Failed to popen rc file %s", rc_path);
    }

    // Read output
    data = NULL;
    data_len = 0;
    data_cap = 0;
    while ((nbytes = fread(buf, 1, sizeof(buf), fp)) != 0) {
        if (data_len + nbytes >= data_cap) {
            data_cap += sizeof(buf);
            data = realloc(data, data_cap);
        }
        memmove(data + data_len, buf, nbytes);
        data_len += nbytes;
    }

    // Add null terminator
    if (data_len + 1 >= data_cap) {
        data_cap += 1;
        data = realloc(data, data_cap);
    }
    data[data_len] = '\0';

    // Return
    pclose(fp);
    *ret_rc_data = data;
    *ret_rc_data_len = data_len;
    return MLE_OK;
}

// Parse rc file
static int _editor_init_from_rc(editor_t *editor, FILE *rc, char *rc_path) {
    int rv;
    size_t rc_data_len;
    char *rc_data;
    char *rc_data_stop;
    char *eol;
    char *bol;
    int fargc;
    char **fargv;
    struct stat statbuf;
    rv = MLE_OK;
    rc_data = NULL;
    rc_data_len = 0;

    // Read or exec rc file
    if (fstat(fileno(rc), &statbuf) == 0 && statbuf.st_mode & S_IXUSR) {
        _editor_init_from_rc_exec(editor, rc_path, &rc_data, &rc_data_len);
    } else {
        _editor_init_from_rc_read(editor, rc, &rc_data, &rc_data_len);
    }
    rc_data_stop = rc_data + rc_data_len;

    // Make fargc, fargv
    int i;
    fargv = NULL;
    for (i = 0; i < 2; i++) {
        bol = rc_data;
        fargc = 1;
        while (bol < rc_data_stop) {
            eol = strchr(bol, '\n');
            if (!eol) eol = rc_data_stop - 1;
            if (*bol != ';') { // Treat semicolon lines as comments
                if (fargv) {
                    *eol = '\0';
                    fargv[fargc] = bol;
                }
                fargc += 1;
            }
            bol = eol + 1;
        }
        if (!fargv) {
            if (fargc < 2) break; // No options
            fargv = malloc((fargc + 1) * sizeof(char*));
            fargv[0] = "mle";
            fargv[fargc] = NULL;
        }
    }

    // Parse args
    if (fargv) {
        rv = _editor_init_from_args(editor, fargc, fargv);
        free(fargv);
    }

    free(rc_data);

    return rv;
}

// Parse cli args
static int _editor_init_from_args(editor_t *editor, int argc, char **argv) {
    int rv;
    kmap_t *cur_kmap;
    syntax_t *cur_syntax;
    uscript_t *uscript;
    int c;
    rv = MLE_OK;

    cur_kmap = NULL;
    cur_syntax = NULL;
    optind = 1;
    while (rv == MLE_OK && (c = getopt(argc, argv, "ha:b:c:H:i:K:k:l:M:m:Nn:p:S:s:t:u:vw:x:y:z:Q:")) != -1) {
        switch (c) {
            case 'h':
                printf("mle version %s\n\n", MLE_VERSION);
                printf("Usage: mle [options] [file:line]...\n\n");
                printf("    -h           Show this message\n");
                printf("    -a <1|0>     Enable/disable tab_to_space (default: %d)\n", MLE_DEFAULT_TAB_TO_SPACE);
                printf("    -b <1|0>     Enable/disable highlight bracket pairs (default: %d)\n", MLE_DEFAULT_HILI_BRACKET_PAIRS);
                printf("    -c <column>  Color column (default: -1, disabled)\n");
                printf("    -H <1|0>     Enable/disable headless mode (default: 1 if no tty, else 0)\n");
                printf("    -i <1|0>     Enable/disable auto_indent (default: %d)\n", MLE_DEFAULT_AUTO_INDENT);
                printf("    -K <kdef>    Make a kmap definition (use with -k)\n");
                printf("    -k <kbind>   Add key binding to current kmap definition (use after -K)\n");
                printf("    -l <ltype>   Set linenum type (default: 0, absolute)\n");
                printf("    -M <macro>   Add a macro\n");
                printf("    -m <key>     Set macro toggle key (default: %s)\n", MLE_DEFAULT_MACRO_TOGGLE_KEY);
                printf("    -N           Skip reading of rc file\n");
                printf("    -n <kmap>    Set init kmap (default: mle_normal)\n");
                printf("    -p <macro>   Set startup macro\n");
                printf("    -S <syndef>  Make a syntax definition (use with -s)\n");
                printf("    -s <synrule> Add syntax rule to current syntax definition (use after -S)\n");
                printf("    -t <size>    Set tab size (default: %d)\n", MLE_DEFAULT_TAB_WIDTH);
                printf("    -u <1|0>     Set coarse undo/redo (default: %d)\n", MLE_DEFAULT_COARSE_UNDO);
                printf("    -v           Print version and exit\n");
                printf("    -w <1|0>     Enable/disable soft word wrap (default: %d)\n", MLE_DEFAULT_SOFT_WRAP);
                printf("    -x <uscript> Run a Lua user script\n");
                printf("    -y <syntax>  Set override syntax for files opened at start up\n");
                printf("    -z <1|0>     Enable/disable trim_paste (default: %d)\n", MLE_DEFAULT_TRIM_PASTE);
                printf("\n");
                printf("    file         At start up, open file\n");
                printf("    file:line    At start up, open file at line\n");
                printf("    kdef         '<name>,<default_cmd>,<allow_fallthru>'\n");
                printf("    kbind        '<cmd>,<key>,<param>'\n");
                printf("    ltype        0=absolute, 1=relative, 2=both\n");
                printf("    macro        '<name> <key1> <key2> ... <keyN>'\n");
                printf("    syndef       '<name>,<path_pattern>,<tab_width>,<tab_to_space>'\n");
                printf("    synrule      '<start>,<end>,<fg>,<bg>'\n");
                printf("    fg,bg        0=default     1=black       2=red         3=green\n");
                printf("                 4=yellow      5=blue        6=magenta     7=cyan\n");
                printf("                 8=white       256=bold      512=underline 1024=reverse\n");
                rv = MLE_ERR;
                break;
            case 'a':
                editor->tab_to_space = atoi(optarg) ? 1 : 0;
                break;
            case 'b':
                editor->highlight_bracket_pairs = atoi(optarg) ? 1 : 0;
                break;
            case 'c':
                editor->color_col = atoi(optarg);
                break;
            case 'H':
                editor->headless_mode = atoi(optarg) ? 1 : 0;
                break;
            case 'i':
                editor->auto_indent = atoi(optarg) ? 1 : 0;
                break;
            case 'K':
                if (_editor_init_kmap_by_str(editor, &cur_kmap, optarg) != MLE_OK) {
                    MLE_LOG_ERR("Could not init kmap by str: %s\n", optarg);
                    editor->exit_code = EXIT_FAILURE;
                    rv = MLE_ERR;
                }
                break;
            case 'k':
                if (!cur_kmap || _editor_init_kmap_add_binding_by_str(editor, cur_kmap, optarg) != MLE_OK) {
                    MLE_LOG_ERR("Could not add key binding to kmap %p by str: %s\n", (void*)cur_kmap, optarg);
                    editor->exit_code = EXIT_FAILURE;
                    rv = MLE_ERR;
                }
                break;
            case 'l':
                editor->linenum_type = atoi(optarg);
                if (editor->linenum_type < 0 || editor->linenum_type > 2) editor->linenum_type = 0;
                break;
            case 'M':
                if (_editor_add_macro_by_str(editor, optarg) != MLE_OK) {
                    MLE_LOG_ERR("Could not add macro by str: %s\n", optarg);
                    editor->exit_code = EXIT_FAILURE;
                    rv = MLE_ERR;
                }
                break;
            case 'm':
                if (_editor_set_macro_toggle_key(editor, optarg) != MLE_OK) {
                    MLE_LOG_ERR("Could not set macro key to: %s\n", optarg);
                    editor->exit_code = EXIT_FAILURE;
                    rv = MLE_ERR;
                }
                break;
            case 'N':
                // See _editor_should_skip_rc
                break;
            case 'n':
                if (editor->kmap_init_name) free(editor->kmap_init_name);
                editor->kmap_init_name = strdup(optarg);
                break;
            case 'p':
                if (editor->startup_macro_name) free(editor->startup_macro_name);
                editor->startup_macro_name = strdup(optarg);
                break;
            case 'S':
                if (_editor_init_syntax_by_str(editor, &cur_syntax, optarg) != MLE_OK) {
                    MLE_LOG_ERR("Could not init syntax by str: %s\n", optarg);
                    editor->exit_code = EXIT_FAILURE;
                    rv = MLE_ERR;
                }
                break;
            case 's':
                if (!cur_syntax || _editor_init_syntax_add_rule_by_str(cur_syntax, optarg) != MLE_OK) {
                    MLE_LOG_ERR("Could not add style rule to syntax %p by str: %s\n", (void*)cur_syntax, optarg);
                    editor->exit_code = EXIT_FAILURE;
                    rv = MLE_ERR;
                }
                break;
            case 't':
                editor->tab_width = atoi(optarg);
                break;
            case 'u':
                editor->coarse_undo = atoi(optarg);
                break;
            case 'v':
                printf("mle version %s\n", MLE_VERSION);
                rv = MLE_ERR;
                break;
            case 'w':
                editor->soft_wrap = atoi(optarg);
                break;
            case 'x':
                if (!(uscript = uscript_run(editor, optarg))) {
                    MLE_LOG_ERR("Failed to run uscript: %s\n", optarg);
                    editor->exit_code = EXIT_FAILURE;
                    rv = MLE_ERR;
                } else {
                    DL_APPEND(editor->uscripts, uscript);
                }
                break;
            case 'y':
                editor->syntax_override = strcmp(optarg, "-") == 0 ? "syn_generic" : optarg;
                break;
            case 'z':
                editor->trim_paste = atoi(optarg) ? 1 : 0;
                break;
            case 'Q':
                switch (*optarg) {
                    case 'q': editor->debug_exit_after_startup = 1; break;
                    case 'd': editor->debug_dump_state_on_exit = 1; break;
                }
                break;
            default: // Unknown option
                editor->exit_code = EXIT_FAILURE;
                rv = MLE_ERR;
                break;
        }
    }

    return rv;
}

// Init status bar
static void _editor_init_status(editor_t *editor) {
    editor->status = bview_new(editor, MLE_BVIEW_TYPE_STATUS, NULL, 0, NULL);
    editor->rect_status.fg = TB_WHITE;
    editor->rect_status.bg = TB_BLACK;
}

// Init bviews
static void _editor_init_bviews(editor_t *editor, int argc, char **argv) {
    int i;
    char *path;
    int path_len;

    // Open bviews
    if (optind >= argc) {
        // Open blank
        editor_open_bview(editor, NULL, MLE_BVIEW_TYPE_EDIT, NULL, 0, 1, 0, 0, NULL, NULL);
    } else {
        // Open files
        for (i = optind; i < argc; i++) {
            path = argv[i];
            path_len = strlen(path);
            editor_open_bview(editor, NULL, MLE_BVIEW_TYPE_EDIT, path, path_len, 1, 0, 0, NULL, NULL);
        }
    }
}

// Init headless mode
static int _editor_init_headless_mode(editor_t *editor) {
    fd_set readfds;
    ssize_t nbytes;
    char buf[1024];
    bview_t *bview;
    int stdin_is_a_pipe;

    // Check if stdin is a pipe
    stdin_is_a_pipe = isatty(STDIN_FILENO) != 1 ? 1 : 0;

    // Bail if not in headless mode and stdin is not a pipe
    if (!editor->headless_mode && !stdin_is_a_pipe) return MLE_OK;

    // Ensure blank bview
    if (!editor->active_edit->buffer->path
        && editor->active_edit->buffer->byte_count == 0
    ) {
        bview = editor->active_edit;
    } else {
        editor_open_bview(editor, NULL, MLE_BVIEW_TYPE_EDIT, NULL, 0, 1, 0, 0, NULL, &bview);
    }

    // If stdin is a pipe, read into bview
    if (!stdin_is_a_pipe) return MLE_OK;
    do {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        select(STDIN_FILENO + 1, &readfds, NULL, NULL, NULL);
        nbytes = 0;
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            nbytes = read(STDIN_FILENO, &buf, 1024);
            if (nbytes > 0) {
                mark_insert_before(bview->active_cursor->mark, buf, nbytes);
            }
        }
    } while (nbytes > 0);

    mark_move_beginning(bview->active_cursor->mark);
    return MLE_OK;
}

// Init startup macro if present
static int _editor_init_startup_macro(editor_t *editor) {
    kmacro_t *macro;
    if (!editor->startup_macro_name) return MLE_OK;
    macro = NULL;
    HASH_FIND_STR(editor->macro_map, editor->startup_macro_name, macro);
    if (!macro) return MLE_ERR;
    editor->macro_apply = macro;
    editor->macro_apply_input_index = 0;
    return MLE_OK;
}
