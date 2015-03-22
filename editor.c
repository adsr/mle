#include <unistd.h>
#include <signal.h>
#include <termbox.h>
#include "uthash.h"
#include "utlist.h"
#include "mle.h"
#include "mlbuf.h"

static int _editor_bview_exists_inner(bview_t* parent, bview_t* needle);
static int _editor_bview_edit_count_inner(bview_t* bview);
static int _editor_close_bview_inner(editor_t* editor, bview_t* bview);
static int _editor_prompt_input_submit(cmd_context_t* ctx);
static int _editor_prompt_yn_yes(cmd_context_t* ctx);
static int _editor_prompt_yn_no(cmd_context_t* ctx);
static int _editor_prompt_cancel(cmd_context_t* ctx);
static void _editor_startup(editor_t* editor);
static void _editor_loop(editor_t* editor, loop_context_t* loop_ctx);
static int _editor_maybe_toggle_macro(editor_t* editor, kinput_t* input);
static void _editor_resize(editor_t* editor, int w, int h);
static void _editor_draw_cursors(editor_t* editor, bview_t* bview);
static void _editor_display(editor_t* editor);
static void _editor_get_input(editor_t* editor, kinput_t* ret_input);
static void _editor_get_user_input(editor_t* editor, kinput_t* ret_input);
static void _editor_record_macro_input(editor_t* editor, kinput_t* input);
static cmd_func_t _editor_get_command(editor_t* editor, kinput_t input);
static cmd_func_t _editor_resolve_funcref(editor_t* editor, cmd_funcref_t* ref);
static int _editor_key_to_input(char* key, kinput_t* ret_input);
static void _editor_init_signal_handlers(editor_t* editor);
static void _editor_graceful_exit(int signum);
static void _editor_init_kmaps(editor_t* editor);
static void _editor_init_kmap(editor_t* editor, kmap_t** ret_kmap, char* name, cmd_funcref_t default_funcref, int allow_fallthru, kbinding_def_t* defs);
static void _editor_init_kmap_add_binding(kmap_t* kmap, kbinding_def_t def);
static int _editor_init_kmap_by_str(editor_t* editor, kmap_t** ret_kmap, char* str);
static int _editor_init_kmap_add_binding_by_str(kmap_t* kmap, char* str);
static void _editor_destroy_kmap(kmap_t* kmap);
static void _editor_init_syntaxes(editor_t* editor);
static void _editor_init_syntax(editor_t* editor, syntax_t** optret_syntax, char* name, char* path_pattern, srule_def_t* defs);
static int _editor_init_syntax_by_str(editor_t* editor, syntax_t** ret_syntax, char* str);
static void _editor_init_syntax_add_rule(syntax_t* syntax, srule_def_t def);
static int _editor_init_syntax_add_rule_by_str(syntax_t* syntax, char* str);
static void _editor_destroy_syntax_map(syntax_t* map);
static void _editor_init_from_rc(editor_t* editor, FILE* rc);
static void _editor_init_from_args(editor_t* editor, int argc, char** argv);
static void _editor_init_status(editor_t* editor);
static void _editor_init_bviews(editor_t* editor, int argc, char** argv);

// Init editor from args
int editor_init(editor_t* editor, int argc, char** argv) {
    FILE* rc;
    char *home_rc;

    // Set editor defaults
    editor->is_in_init = 1;
    editor->tab_width = MLE_DEFAULT_TAB_WIDTH;
    editor->tab_to_space = MLE_DEFAULT_TAB_TO_SPACE;
    editor->viewport_scope_x = -4;
    editor->viewport_scope_y = -4;
    editor->startup_linenum = -1;
    editor_set_macro_toggle_key(editor, MLE_DEFAULT_MACRO_TOGGLE_KEY);

    // Init signal handlers
    _editor_init_signal_handlers(editor);

    // Init kmaps
    _editor_init_kmaps(editor);

    // Init syntaxes
    _editor_init_syntaxes(editor);

    // Parse rc files
    home_rc = NULL;
    if (getenv("HOME")) {
        asprintf(&home_rc, "%s/%s", getenv("HOME"), ".mlerc");
        if (util_file_exists(home_rc, "rb", &rc)) {
            _editor_init_from_rc(editor, rc);
        }
        free(home_rc);
    }
    if (util_file_exists("/etc/mlerc", "rb", &rc)) {
        _editor_init_from_rc(editor, rc);
    }

    // Parse cli args
    _editor_init_from_args(editor, argc, argv);

    // Init status bar
    _editor_init_status(editor);

    // Init bviews
    _editor_init_bviews(editor, argc, argv);

    editor->is_in_init = 0;
    return MLE_OK;
}

// Run editor
int editor_run(editor_t* editor) {
    loop_context_t loop_ctx;
    loop_ctx.should_exit = 0;
    _editor_resize(editor, -1, -1);
    _editor_startup(editor);
    _editor_loop(editor, &loop_ctx);
    return MLE_OK;
}

// Deinit editor
int editor_deinit(editor_t* editor) {
    bview_t* bview;
    bview_t* bview_tmp;
    kmap_t* kmap;
    kmap_t* kmap_tmp;
    bview_destroy(editor->status);
    DL_FOREACH_SAFE(editor->bviews, bview, bview_tmp) {
        DL_DELETE(editor->bviews, bview);
        bview_destroy(bview);
    }
    HASH_ITER(hh, editor->kmap_map, kmap, kmap_tmp) {
        HASH_DEL(editor->kmap_map, kmap);
        _editor_destroy_kmap(kmap);
    }
    _editor_destroy_syntax_map(editor->syntax_map);
    return MLE_OK;
}

// Prompt user for input
int editor_prompt(editor_t* editor, char* prompt_key, char* label, char* opt_data, int opt_data_len, kmap_t* opt_kmap, char** optret_answer) {
    loop_context_t loop_ctx;

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
    editor_open_bview(editor, MLE_BVIEW_TYPE_PROMPT, NULL, 0, 1, &editor->rect_prompt, NULL, &editor->prompt);
    editor->prompt->prompt_key = prompt_key;
    editor->prompt->prompt_label = label;
    bview_push_kmap(editor->prompt, opt_kmap ? opt_kmap : editor->kmap_prompt_input);

    // Insert opt_data if present
    if (opt_data && opt_data_len > 0) {
        buffer_insert(editor->prompt->buffer, 0, opt_data, opt_data_len, NULL);
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
    editor_close_bview(editor, editor->prompt);
    editor_set_active(editor, loop_ctx.invoker);
    editor->prompt = NULL;

    return MLE_OK;
}

// Open a bview
int editor_open_bview(editor_t* editor, int type, char* opt_path, int opt_path_len, int make_active, bview_rect_t* opt_rect, buffer_t* opt_buffer, bview_t** optret_bview) {
    bview_t* bview;
    bview = bview_new(editor, opt_path, opt_path_len, opt_buffer);
    bview->type = type;
    DL_APPEND(editor->bviews, bview);
    editor->bviews_tail = bview;
    if (make_active) {
        editor_set_active(editor, bview);
    }
    if (opt_rect) {
        bview_resize(bview, opt_rect->x, opt_rect->y, opt_rect->w, opt_rect->h);
    }
    if (optret_bview) {
        *optret_bview = bview;
    }
    return MLE_OK;
}

// Close a bview
int editor_close_bview(editor_t* editor, bview_t* bview) {
    int rc;
    if ((rc = _editor_close_bview_inner(editor, bview)) == MLE_OK) {
        _editor_resize(editor, editor->w, editor->h);
    }
    return rc;
}

// Set the active bview
int editor_set_active(editor_t* editor, bview_t* bview) {
    if (!editor_bview_exists(editor, bview)) {
        MLE_RETURN_ERR("No bview %p in editor->bviews\n", bview);
    }
    editor->active = bview;
    if (MLE_BVIEW_IS_EDIT(bview)) {
        editor->active_edit = bview;
        editor->active_edit_root = bview_get_split_root(bview);
    }
    bview_rectify_viewport(bview);
    return MLE_OK;
}

// Set macro toggle key
int editor_set_macro_toggle_key(editor_t* editor, char* key) {
    return _editor_key_to_input(key, &editor->macro_toggle_key);
}

// Return 1 if bview exists in editor, else return 0
int editor_bview_exists(editor_t* editor, bview_t* bview) {
    bview_t* tmp;
    DL_FOREACH(editor->bviews, tmp) {
        if (_editor_bview_exists_inner(tmp, bview)) {
            return 1;
        }
    }
    raise(SIGINT);
    return 0;
}

// Return number of EDIT bviews open
int editor_bview_edit_count(editor_t* editor) {
    int count;
    bview_t* bview;
    count = 0;
    DL_FOREACH(editor->bviews, bview) {
        if (MLE_BVIEW_IS_EDIT(bview)) {
            count += _editor_bview_edit_count_inner(bview);
        }
    }
    return count;
}

// Return 1 if parent or a child of parent == needle, otherwise return 0
static int _editor_bview_exists_inner(bview_t* parent, bview_t* needle) {
    if (parent == needle) {
        return 1;
    }
    if (parent->split_child) {
        return _editor_bview_exists_inner(parent->split_child, needle);
    }
    return 0;
}

// Return number of edit bviews open including split children
static int _editor_bview_edit_count_inner(bview_t* bview) {
    if (bview->split_child) {
        return 1 + _editor_bview_edit_count_inner(bview->split_child);
    }
    return 1;
}

// Close a bview
static int _editor_close_bview_inner(editor_t* editor, bview_t* bview) {
    if (!editor_bview_exists(editor, bview)) {
        MLE_RETURN_ERR("No bview %p in editor->bviews\n", bview);
    }
    if (bview->split_child) {
        _editor_close_bview_inner(editor, bview->split_child);
    }
    DL_DELETE(editor->bviews, bview);
    if (bview->split_parent) {
        bview->split_parent->split_child = NULL;
        editor_set_active(editor, bview->split_parent);
    } else { 
        if (bview->prev && bview->prev != bview && MLE_BVIEW_IS_EDIT(bview->prev)) {
            editor_set_active(editor, bview->prev);
        } else if (bview->next && MLE_BVIEW_IS_EDIT(bview->next)) {
            editor_set_active(editor, bview->next);
        } else {
            editor_open_bview(editor, MLE_BVIEW_TYPE_EDIT, NULL, 0, 1, &editor->rect_edit, NULL, NULL);
        }
    }
    bview_destroy(bview);
    return MLE_OK;
}

// Invoked when user hits enter in a prompt_input
static int _editor_prompt_input_submit(cmd_context_t* ctx) {
    bint_t answer_len;
    char* answer;
    buffer_get(ctx->bview->buffer, &answer, &answer_len);
    ctx->loop_ctx->prompt_answer = strndup(answer, answer_len);
    ctx->loop_ctx->should_exit = 1;
    return MLE_OK;
}

// Invoked when user hits y in a prompt_yn
static int _editor_prompt_yn_yes(cmd_context_t* ctx) {
    ctx->loop_ctx->prompt_answer = MLE_PROMPT_YES;
    ctx->loop_ctx->should_exit = 1;
    return MLE_OK;
}

// Invoked when user hits n in a prompt_yn
static int _editor_prompt_yn_no(cmd_context_t* ctx) {
    ctx->loop_ctx->prompt_answer = MLE_PROMPT_NO;
    ctx->loop_ctx->should_exit = 1;
    return MLE_OK;
}

// Invoked when user cancels (Ctrl-C) a prompt_(input|yn), or hits any key in a prompt_ok
static int _editor_prompt_cancel(cmd_context_t* ctx) {
    ctx->loop_ctx->prompt_answer = NULL;
    ctx->loop_ctx->should_exit = 1;
    return MLE_OK;
}

// Run startup actions. This is before any user-input is processed.
static void _editor_startup(editor_t* editor) {
    // Jump to line in current bview if specified
    if (editor->startup_linenum >= 0) {
        mark_move_to(editor->active_edit->active_cursor->mark, editor->startup_linenum, 0);
        bview_center_viewport_y(editor->active_edit);
    }
}

// Run editor loop
static void _editor_loop(editor_t* editor, loop_context_t* loop_ctx) {
    cmd_context_t cmd_ctx;
    cmd_func_t cmd_fn;

    // Init cmd_context
    memset(&cmd_ctx, 0, sizeof(cmd_context_t));
    cmd_ctx.editor = editor;
    cmd_ctx.loop_ctx = loop_ctx;

    // Loop until editor should exit
    while (!loop_ctx->should_exit) {

        // Display editor
        if (!editor->is_display_disabled) {
            _editor_display(editor);
        }

        // Get input
        _editor_get_input(editor, &cmd_ctx.input);

        // Toggle macro?
        if (_editor_maybe_toggle_macro(editor, &cmd_ctx.input) == 1) {
            continue;
        }

        // Execute command
        if ((cmd_fn = _editor_get_command(editor, cmd_ctx.input)) != NULL) {
            cmd_ctx.cursor = editor->active ? editor->active->active_cursor : NULL;
            cmd_ctx.bview = cmd_ctx.cursor ? cmd_ctx.cursor->bview : NULL;
            cmd_fn(&cmd_ctx);
        }
    }
}

// If input == editor->macro_toggle_key, toggle macro mode and return 1
static int _editor_maybe_toggle_macro(editor_t* editor, kinput_t* input) {
    if (memcmp(input, &editor->macro_toggle_key, sizeof(kinput_t)) == 0) {
        if (editor->is_recording_macro) {
            // Stop recording macro and add to map
            HASH_ADD(hh, editor->macro_map, key, sizeof(kinput_t), editor->macro_record);
            editor->macro_record = NULL;
            editor->is_recording_macro = 0;
        } else {
            // Get macro_key and start recording
            editor->macro_record = calloc(1, sizeof(kmacro_t));
            _editor_get_input(editor, &editor->macro_record->key);
            editor->is_recording_macro = 1;
        }
        return 1;
    }
    return 0;
}

// Resize the editor
static void _editor_resize(editor_t* editor, int w, int h) {
    bview_t* bview;
    bview_rect_t* bounds;

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

    editor->rect_popup.x = 0;
    editor->rect_popup.y = editor->h - 2 - (editor->popup_h);
    editor->rect_popup.w = editor->w;
    editor->rect_popup.h = editor->popup_h;

    DL_FOREACH(editor->bviews, bview) {
        if (MLE_BVIEW_IS_PROMPT(bview)) {
            bounds = &editor->rect_prompt;
        } else if (MLE_BVIEW_IS_POPUP(bview)) {
            bounds = &editor->rect_popup;
        } else if (MLE_BVIEW_IS_STATUS(bview)) {
            bounds = &editor->rect_status;
        } else {
            if (bview->split_parent) continue;
            bounds = &editor->rect_edit;
        }
        bview_resize(bview, bounds->x, bounds->y, bounds->w, bounds->h);
    }
}

// Draw bviews cursors recursively
static void _editor_draw_cursors(editor_t* editor, bview_t* bview) {
    if (MLE_BVIEW_IS_EDIT(bview) && bview_get_split_root(bview) != editor->active_edit_root) {
        return;
    }
    bview_draw_cursor(bview, bview == editor->active ? 1 : 0);
    if (bview->split_child) {
        _editor_draw_cursors(editor, bview->split_child);
    }
}

// Display the editor
static void _editor_display(editor_t* editor) {
    bview_t* bview;
    tb_clear();
    bview_draw(editor->active_edit_root);
    bview_draw(editor->status);
    if (editor->popup) bview_draw(editor->popup);
    if (editor->prompt) bview_draw(editor->prompt);
    DL_FOREACH(editor->bviews, bview) _editor_draw_cursors(editor, bview);
    tb_present();
}

// Get input from either macro or user
static void _editor_get_input(editor_t* editor, kinput_t* ret_input) {
    while (1) {
        if (editor->macro_apply
            && editor->macro_apply_input_index < editor->macro_apply->inputs_len
        ) {
            // Get input from macro
            *ret_input = editor->macro_apply->inputs[editor->macro_apply_input_index];
            editor->macro_apply_input_index += 1;
        } else {
            // Clear macro
            if (editor->macro_apply) {
                editor->macro_apply = NULL;
                editor->macro_apply_input_index = 0;
            }
            // Get user input
            _editor_get_user_input(editor, ret_input);
        }
        if (editor->is_recording_macro) {
            // Record macro input
            _editor_record_macro_input(editor, ret_input);
        }
        break;
    }
}

// Get user input
static void _editor_get_user_input(editor_t* editor, kinput_t* ret_input) {
    int rc;
    tb_event_t ev;
    while (1) {
        rc = tb_poll_event(&ev);
        if (rc == -1) {
            continue;
        } else if (rc == TB_EVENT_RESIZE) {
            _editor_resize(editor, ev.w, ev.h);
            _editor_display(editor);
            continue;
        }
        *ret_input = (kinput_t){ ev.mod, ev.ch, ev.key };
        break;
    }
}

// Copy input into macro buffer
static void _editor_record_macro_input(editor_t* editor, kinput_t* input) {
    kmacro_t* macro;
    macro = editor->macro_record;
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
static cmd_func_t _editor_get_command(editor_t* editor, kinput_t input) {
    kmap_node_t* kmap_node;
    kbinding_t tmp_binding;
    kbinding_t* binding;

    kmap_node = editor->active->kmap_tail;
    memset(&tmp_binding, 0, sizeof(kbinding_t));
    tmp_binding.input = input;
    binding = NULL;
    while (kmap_node) {
        HASH_FIND(hh, kmap_node->kmap->bindings, &tmp_binding.input, sizeof(kinput_t), binding);
        if (binding) {
            return _editor_resolve_funcref(editor, &binding->funcref);
        } else if (kmap_node->kmap->default_funcref) {
            return _editor_resolve_funcref(editor, kmap_node->kmap->default_funcref);
        }
        if (kmap_node->kmap->allow_fallthru) {
            kmap_node = kmap_node->prev;
        } else {
            kmap_node = NULL;
        }
    }
    return NULL;
}

// Resolve a funcref to a func
static cmd_func_t _editor_resolve_funcref(editor_t* editor, cmd_funcref_t* ref) {
    cmd_funcref_t* resolved;
    if (!ref->func) {
        HASH_FIND_STR(editor->func_map, ref->name, resolved);
        if (resolved) {
            ref->func = resolved->func;
        }
    }
    return ref->func;
}

// Return a kinput_t given a key name
static int _editor_key_to_input(char* key, kinput_t* ret_input) {
    int keylen;
    int mod;
    uint32_t ch;
    keylen = strlen(key);

    // Check for special key
#define MLE_KEY_DEF(pckey, pmod, pch, pkey) \
    } else if (!strncmp((pckey), key, keylen)) { \
        *ret_input = (kinput_t){ (pmod), 0, (pkey) }; \
        return MLE_OK;
    if (keylen < 1) {
        MLE_RETURN_ERR("key has length %d\n", keylen);
#include "keys.h"
    }
#undef MLE_KEY_DEF

    // Check for character, with potential ALT modifier
    mod = 0;
    ch = 0;
    if (keylen > 2 && !strncmp("M-", key, 2)) {
        mod = TB_MOD_ALT;
        key += 2;
    }
    utf8_char_to_unicode(&ch, key, NULL);
    if (ch < 1) {
        return MLE_ERR;
    }
    *ret_input = (kinput_t){ mod, ch, 0 };
    return MLE_OK;
}

// Init signal handlers
static void _editor_init_signal_handlers(editor_t* editor) {
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = _editor_graceful_exit;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGQUIT, &action, NULL);
    sigaction(SIGHUP, &action, NULL);
}

// Gracefully exit
static void _editor_graceful_exit(int signum) {
    bview_t* bview;
    char path[64];
    int bview_num;
    bview_num = 0;
    tb_shutdown();
    DL_FOREACH(_editor.bviews, bview) {
        snprintf((char*)&path, 64, "mle.bak.%d.%d", getpid(), bview_num);
        buffer_save_as(bview->buffer, path, strlen(path));
        bview_num += 1;
    }
    editor_deinit(&_editor);
    exit(1);
}

// Init built-in kmaps
static void _editor_init_kmaps(editor_t* editor) {
    _editor_init_kmap(editor, &editor->kmap_normal, "mle_normal", MLE_FUNCREF(cmd_insert_data), 0, (kbinding_def_t[]){
        MLE_KBINDING_DEF(cmd_insert_tab, "tab"),
        MLE_KBINDING_DEF(cmd_insert_newline, "enter"),
        MLE_KBINDING_DEF(cmd_delete_before, "backspace"),
        MLE_KBINDING_DEF(cmd_delete_before, "backspace2"),
        MLE_KBINDING_DEF(cmd_delete_after, "delete"),
        MLE_KBINDING_DEF(cmd_move_bol, "C-a"),
        MLE_KBINDING_DEF(cmd_move_bol, "home"),
        MLE_KBINDING_DEF(cmd_move_eol, "C-e"),
        MLE_KBINDING_DEF(cmd_move_eol, "end"),
        MLE_KBINDING_DEF(cmd_move_beginning, "M-\\"),
        MLE_KBINDING_DEF(cmd_move_end, "M-/"),
        MLE_KBINDING_DEF(cmd_move_left, "left"),
        MLE_KBINDING_DEF(cmd_move_right, "right"),
        MLE_KBINDING_DEF(cmd_move_up, "up"),
        MLE_KBINDING_DEF(cmd_move_down, "down"),
        MLE_KBINDING_DEF(cmd_move_page_up, "page-up"),
        MLE_KBINDING_DEF(cmd_move_page_down, "page-down"),
        MLE_KBINDING_DEF(cmd_move_to_line, "M-g"),
        MLE_KBINDING_DEF(cmd_move_word_forward, "M-f"),
        MLE_KBINDING_DEF(cmd_move_word_back, "M-b"),
        MLE_KBINDING_DEF(cmd_toggle_sel_bound, "M-a"),
        MLE_KBINDING_DEF(cmd_drop_sleeping_cursor, "M-h"),
        MLE_KBINDING_DEF(cmd_wake_sleeping_cursors, "M-j"),
        MLE_KBINDING_DEF(cmd_remove_extra_cursors, "M-k"),
        MLE_KBINDING_DEF(cmd_search, "C-f"),
        MLE_KBINDING_DEF(cmd_search_next, "C-j"),
        MLE_KBINDING_DEF(cmd_replace, "M-r"),
        MLE_KBINDING_DEF(cmd_isearch, "C-r"),
        MLE_KBINDING_DEF(cmd_delete_word_before, "C-w"),
        MLE_KBINDING_DEF(cmd_delete_word_after, "M-d"),
        MLE_KBINDING_DEF(cmd_cut, "C-k"),
        MLE_KBINDING_DEF(cmd_copy, "C-c"),
        MLE_KBINDING_DEF(cmd_uncut, "C-u"),
        MLE_KBINDING_DEF(cmd_next, "M-n"),
        MLE_KBINDING_DEF(cmd_prev, "M-p"),
        MLE_KBINDING_DEF(cmd_split_vertical, "M-l"),
        MLE_KBINDING_DEF(cmd_split_horizontal, "M-;"),
        MLE_KBINDING_DEF(cmd_save, "C-o"),
        MLE_KBINDING_DEF(cmd_new, "C-n"),
        MLE_KBINDING_DEF(cmd_new_open, "C-b"),
        MLE_KBINDING_DEF(cmd_replace_new, "C-p"),
        MLE_KBINDING_DEF(cmd_replace_open, "C-l"),
        MLE_KBINDING_DEF(cmd_close, "M-c"),
        MLE_KBINDING_DEF(cmd_quit, "C-x"),
        MLE_KBINDING_DEF(NULL, "")
    });
    _editor_init_kmap(editor, &editor->kmap_prompt_input, "mle_prompt_input", MLE_FUNCREF(NULL), 1, (kbinding_def_t[]){
        MLE_KBINDING_DEF(_editor_prompt_input_submit, "enter"),
        MLE_KBINDING_DEF(_editor_prompt_cancel, "C-c"),
        MLE_KBINDING_DEF(NULL, "")
    });
    _editor_init_kmap(editor, &editor->kmap_prompt_yn, "mle_prompt_yn", MLE_FUNCREF(NULL), 0, (kbinding_def_t[]){
        MLE_KBINDING_DEF(_editor_prompt_yn_yes, "y"),
        MLE_KBINDING_DEF(_editor_prompt_yn_no, "n"),
        MLE_KBINDING_DEF(_editor_prompt_cancel, "C-c"),
        MLE_KBINDING_DEF(NULL, "")
    });
    _editor_init_kmap(editor, &editor->kmap_prompt_ok, "mle_prompt_ok", MLE_FUNCREF(_editor_prompt_cancel), 0, (kbinding_def_t[]){
        MLE_KBINDING_DEF(NULL, "")
    });
}

// Init a single kmap
static void _editor_init_kmap(editor_t* editor, kmap_t** ret_kmap, char* name, cmd_funcref_t default_funcref, int allow_fallthru, kbinding_def_t* defs) {
    kmap_t* kmap;

    kmap = calloc(1, sizeof(kmap_t));
    snprintf(kmap->name, MLE_KMAP_NAME_MAX_LEN, "%s", name);
    kmap->allow_fallthru = allow_fallthru;
    if (default_funcref.func) {
        kmap->default_funcref = malloc(sizeof(cmd_funcref_t));
        memcpy(kmap->default_funcref, &default_funcref, sizeof(cmd_funcref_t));
    }

    while (defs && defs->funcref.func) {
        _editor_init_kmap_add_binding(kmap, *defs);
        defs++;
    }

    HASH_ADD_KEYPTR(hh, editor->kmap_map, name, strlen(name), kmap);
    *ret_kmap = kmap;
}

// Add a binding to a kmap
static void _editor_init_kmap_add_binding(kmap_t* kmap, kbinding_def_t def) {
    kbinding_t* binding;

    binding = calloc(1, sizeof(kbinding_t));
    binding->funcref = def.funcref;

    if (_editor_key_to_input(def.key, &binding->input) != MLE_OK) {
        free(binding);
        return;
    }

    HASH_ADD(hh, kmap->bindings, input, sizeof(kinput_t), binding);
}

// Proxy for _editor_init_kmap with str in format '<name>,<default_cmd>,<allow_fallthru>'
static int _editor_init_kmap_by_str(editor_t* editor, kmap_t** ret_kmap, char* str) {
    char* args[3];
    args[0] = strtok(str,  ","); if (!args[0]) return MLE_ERR;
    args[1] = strtok(NULL, ","); if (!args[1]) return MLE_ERR;
    args[2] = strtok(NULL, ","); if (!args[2]) return MLE_ERR;
    _editor_init_kmap(editor, ret_kmap, args[0], (cmd_funcref_t){ args[1], NULL, NULL }, atoi(args[2]), NULL);
    return MLE_OK;
}

// Proxy for _editor_init_kmap_add_binding with str in format '<cmd>,<key>'
static int _editor_init_kmap_add_binding_by_str(kmap_t* kmap, char* str) {
    char* args[2];
    args[0] = strtok(str,  ","); if (!args[0]) return MLE_ERR;
    args[1] = strtok(NULL, ","); if (!args[1]) return MLE_ERR;
    _editor_init_kmap_add_binding(kmap, (kbinding_def_t){ (cmd_funcref_t){ args[0], NULL, NULL }, args[1] });
    return MLE_OK;
}


// Destroy a kmap
static void _editor_destroy_kmap(kmap_t* kmap) {
    kbinding_t* binding;
    kbinding_t* binding_tmp;
    HASH_ITER(hh, kmap->bindings, binding, binding_tmp) {
        HASH_DELETE(hh, kmap->bindings, binding);
        free(binding);
    }
    if (kmap->default_funcref) free(kmap->default_funcref);
    free(kmap);
}

// Init built-in syntax map
static void _editor_init_syntaxes(editor_t* editor) {
    /*
    _editor_init_syntax(editor, NULL, "mle_generic", "\\.(c|cpp|h|hpp|php|py|rb|sh|pl|go|js|java|lua)$", (srule_def_t[]){
        { "(?<![\\w%@$])("
          "abstract|alias|alignas|alignof|and|and_eq|arguments|array|as|asm|"
          "assert|auto|base|begin|bitand|bitor|bool|boolean|break|byte|"
          "callable|case|catch|chan|char|checked|class|clone|cmp|compl|const|"
          "const_cast|constexpr|continue|debugger|decimal|declare|decltype|"
          "def|default|defer|defined|del|delegate|delete|die|do|done|double|"
          "dynamic_cast|echo|elif|else|elseif|elsif|empty|end|enddeclare|"
          "endfor|endforeach|endif|endswitch|endwhile|ensure|enum|eq|esac|"
          "eval|event|except|exec|exit|exp|explicit|export|extends|extern|"
          "fallthrough|false|fi|final|finally|fixed|float|for|foreach|friend|"
          "from|func|function|ge|global|go|goto|gt|if|implements|implicit|"
          "import|in|include|include_once|inline|instanceof|insteadof|int|"
          "interface|internal|is|isset|lambda|le|let|list|lock|long|lt|m|map|"
          "module|mutable|namespace|native|ne|new|next|nil|no|noexcept|not|"
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
        { "'([^']|\\')*'", NULL, TB_YELLOW | TB_BOLD, TB_DEFAULT },
        { "\"([^\"]|\\\")*\"", NULL, TB_YELLOW | TB_BOLD, TB_DEFAULT },
        { "/([^/]|/)*" "/", NULL, TB_YELLOW, TB_DEFAULT },
        { "/" "/.*$", NULL, TB_CYAN, TB_DEFAULT },
        { "/\\" "*", "\\*" "/", TB_CYAN, TB_DEFAULT },
        { "<\\?(php)?|\\?" ">", NULL, TB_GREEN, TB_DEFAULT },
        { "\\?" ">", "<\\?(php)?", TB_WHITE, TB_DEFAULT },
        { "\"\"\"", "\"\"\"", TB_YELLOW | TB_BOLD, TB_DEFAULT },
        { "\\t+", NULL, TB_DEFAULT, TB_RED },
        { "\\s+$", NULL, TB_DEFAULT, TB_GREEN },
        { NULL, NULL, 0, 0 }
    });
    */
}

// Init a single syntax
static void _editor_init_syntax(editor_t* editor, syntax_t** optret_syntax, char* name, char* path_pattern, srule_def_t* defs) {
    syntax_t* syntax;

    syntax = calloc(1, sizeof(syntax_t));
    snprintf(syntax->name, MLE_SYNTAX_NAME_MAX_LEN, "%s", name);
    syntax->path_pattern = path_pattern;

    while (defs && defs->re) {
        _editor_init_syntax_add_rule(syntax, *defs);
        defs++;
    }
    HASH_ADD_STR(editor->syntax_map, name, syntax);

    if (optret_syntax) *optret_syntax = syntax;
}

// Proxy for _editor_init_syntax with str in format '<name>,<path_pattern>'
static int _editor_init_syntax_by_str(editor_t* editor, syntax_t** ret_syntax, char* str) {
    char* args[2];
    args[0] = strtok(str,  ","); if (!args[0]) return MLE_ERR;
    args[1] = strtok(NULL, ","); if (!args[1]) return MLE_ERR;
    _editor_init_syntax(editor, ret_syntax, args[0], args[1], NULL);
    return MLE_OK;
}

// Add rule to syntax
static void _editor_init_syntax_add_rule(syntax_t* syntax, srule_def_t def) {
    srule_node_t* node;
    node = calloc(1, sizeof(srule_node_t));
    if (def.re_end) {
        node->srule = srule_new_multi(def.re, strlen(def.re), def.re_end, strlen(def.re_end), def.fg, def.bg);
    } else {
        node->srule = srule_new_single(def.re, strlen(def.re), def.fg, def.bg);
    }
    DL_APPEND(syntax->srules, node);
}

// Proxy for _editor_init_syntax_add_rule with str in format '<start>,<end>,<fg>,<bg>' or '<regex>,<fg>,<bg>'
static int _editor_init_syntax_add_rule_by_str(syntax_t* syntax, char* str) {
    char* args[4];
    int style_i;
    args[0] = strtok(str,  ","); if (!args[0]) return MLE_ERR;
    args[1] = strtok(NULL, ","); if (!args[1]) return MLE_ERR;
    args[2] = strtok(NULL, ","); if (!args[2]) return MLE_ERR;
    args[3] = strtok(NULL, ",");
    style_i = args[3] ? 2 : 1;
    _editor_init_syntax_add_rule(syntax, (srule_def_t){ args[0], style_i == 2 ? args[1] : NULL, atoi(args[style_i]), atoi(args[style_i + 1]) });
    return MLE_OK;
}

// Destroy a syntax
static void _editor_destroy_syntax_map(syntax_t* map) {
    syntax_t* syntax;
    syntax_t* syntax_tmp;
    srule_node_t* srule;
    srule_node_t* srule_tmp;
    HASH_ITER(hh, map, syntax, syntax_tmp) {
        HASH_DELETE(hh, map, syntax);
        DL_FOREACH_SAFE(syntax->srules, srule, srule_tmp) {
            DL_DELETE(syntax->srules, srule);
            srule_destroy(srule->srule);
            free(srule);
        }
        free(syntax);
    }
}

// Parse rc file
static void _editor_init_from_rc(editor_t* editor, FILE* rc) {
    long size;
    char *rc_data;
    char *rc_data_stop;
    char* eol;
    char* bol;
    int fargc;
    char** fargv;

    // Read all from rc
    fseek(rc, 0L, SEEK_END);
    size = ftell(rc);
    fseek(rc, 0L, SEEK_SET);
    rc_data = malloc(size + 1);
    fread(rc_data, size, 1, rc);
    rc_data[size] = '\0';
    rc_data_stop = rc_data + size;

    // Make fargc, fargv
    int i;
    fargv = NULL;
    for (i = 0; i < 2; i++) {
        bol = rc_data;
        fargc = 1;
        while (bol < rc_data_stop) {
            eol = strchr(bol, '\n');
            if (!eol) eol = rc_data_stop - 1;
            if (fargv) {
                *eol = '\0';
                fargv[fargc] = bol;
            }
            fargc += 1;
            bol = eol + 1;
        }
        if (!fargv) {
            if (fargc < 2) break;
            fargv = malloc((fargc + 1) * sizeof(char*));
            fargv[0] = "mle";
            fargv[fargc] = NULL;
        }
    }

    // Parse args
    if (fargv) {
        _editor_init_from_args(editor, fargc, fargv);
        free(fargv);
    }

    free(rc_data);
}

// Parse cli args
static void _editor_init_from_args(editor_t* editor, int argc, char** argv) {
    kmap_t* cur_kmap;
    syntax_t* cur_syntax;
    int c;

    cur_kmap = NULL;
    cur_syntax = NULL;
    optind = 0;
    while ((c = getopt(argc, argv, "haK:k:m:n:S:s:t:vx:y:")) != -1) {
        switch (c) {
            case 'h':
                printf("mle version %s\n\n", MLE_VERSION);
                printf("Usage: mle [options] [file:line]...\n\n");
                printf("    -h           Show this message\n");
                printf("    -a           Allow tabs (disable tab-to-space)\n");
                printf("    -K <kdef>    Set current kmap definition (use with -k)\n");
                printf("    -k <kbind>   Add key binding to current kmap definition (use with -K)\n");
                printf("    -m <key>     Set macro toggle key (default: %s)\n", MLE_DEFAULT_MACRO_TOGGLE_KEY);
                printf("    -n <kmap>    Set init kmap (default: mle_normal)\n");
                printf("    -S <syndef>  Set current syntax definition (use with -s)\n");
                printf("    -s <synrule> Add syntax rule to current syntax definition (use with -S)\n");
                printf("    -t <size>    Set tab size (default: %d)\n", MLE_DEFAULT_TAB_WIDTH);
                printf("    -v           Print version and exit\n");
                printf("    -x <script>  Execute user script\n");
                printf("    -y <syntax>  Set override syntax for files opened at start up\n\n");
                printf("    file         At start up, open file\n");
                printf("    file:line    At start up, open file at line\n");
                printf("    kdef         '<name>,<default_cmd>,<allow_fallthru>'\n");
                printf("    kbind        '<cmd>,<key>'\n");
                printf("    syndef       '<name>,<path_pattern>'\n");
                printf("    synrule      '<start>,<end>,<fg>,<bg>'\n");
                exit(EXIT_SUCCESS);
            case 'a':
                editor->tab_to_space = 0;
                break;
            case 'K':
                if (_editor_init_kmap_by_str(editor, &cur_kmap, optarg) != MLE_OK) {
                    MLE_LOG_ERR("Could not init kmap by str: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'k':
                if (!cur_kmap || _editor_init_kmap_add_binding_by_str(cur_kmap, optarg) != MLE_OK) {
                    MLE_LOG_ERR("Could not add key binding to kmap %p by str: %s\n", cur_kmap, optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'm':
                if (editor_set_macro_toggle_key(editor, optarg) != MLE_OK) {
                    MLE_LOG_ERR("Could not set macro key to: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'n':
                editor->kmap_init = optarg;
                break;
            case 'S':
                if (_editor_init_syntax_by_str(editor, &cur_syntax, optarg) != MLE_OK) {
                    MLE_LOG_ERR("Could not init syntax by str: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            case 's':
                if (!cur_syntax || _editor_init_syntax_add_rule_by_str(cur_syntax, optarg) != MLE_OK) {
                    MLE_LOG_ERR("Could not add style rule to syntax %p by str: %s\n", cur_syntax, optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            case 't':
                editor->tab_width = atoi(optarg);
                break;
            case 'v':
                printf("mle version %s\n", MLE_VERSION);
                exit(EXIT_SUCCESS);
            case 'y':
                editor->syntax_override = optarg;
                break;
            default:
                exit(EXIT_FAILURE);
        }
    }
}

// Init status bar
static void _editor_init_status(editor_t* editor) {
    editor->status = bview_new(editor, NULL, 0, NULL);
    editor->status->type = MLE_BVIEW_TYPE_STATUS;
    editor->rect_status.fg = TB_WHITE;
    editor->rect_status.bg = TB_BLACK | TB_BOLD;
}

// Init bviews
static void _editor_init_bviews(editor_t* editor, int argc, char** argv) {
    int i;
    char* colon;
    bview_t* bview;
    char *path;
    int path_len;

    // Open bviews
    if (optind >= argc) {
        // Open blank
        editor_open_bview(editor, MLE_BVIEW_TYPE_EDIT, NULL, 0, 1, &editor->rect_edit, NULL, NULL);
    } else {
        // Open files
        for (i = optind; i < argc; i++) {
            path = argv[i];
            if (util_file_exists(path, NULL, NULL)) {
                editor_open_bview(editor, MLE_BVIEW_TYPE_EDIT, path, path_len, 1, &editor->rect_edit, NULL, NULL);
            } else if ((colon = strrchr(path, ':')) != NULL) {
                path_len = colon - path;
                editor->startup_linenum = strtoul(colon + 1, NULL, 10);
                if (editor->startup_linenum > 0) editor->startup_linenum -= 1;
                editor_open_bview(editor, MLE_BVIEW_TYPE_EDIT, path, path_len, 1, &editor->rect_edit, NULL, &bview);
            } else {
                editor_open_bview(editor, MLE_BVIEW_TYPE_EDIT, path, path_len, 1, &editor->rect_edit, NULL, NULL);
            }
        }
    }
}
