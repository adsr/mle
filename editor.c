#include <unistd.h>
#include <termbox.h>
#include "uthash.h"
#include "utlist.h"
#include "mle.h"
#include "mlbuf.h"

static void _editor_loop(editor_t* editor, loop_context_t* loop_ctx);
static int _editor_maybe_toggle_macro(editor_t* editor, kinput_t* input);
static void _editor_resize(editor_t* editor);
static void _editor_display(editor_t* editor);
static void _editor_get_input(editor_t* editor, kinput_t* ret_input);
static void _editor_get_user_input(editor_t* editor, kinput_t* ret_input);
static void _editor_record_macro_input(editor_t* editor, kinput_t* input);
static cmd_function_t _editor_get_command(editor_t* editor, kinput_t input);
static int _editor_key_to_input(char* key, kinput_t* ret_input);
static void _editor_init_kmaps(editor_t* editor);
static void _editor_init_kmap(kmap_t** ret_kmap, char* name, cmd_function_t default_func, int allow_fallthru, kmap_def_t* defs);
static void _editor_init_syntaxes(editor_t* editor);
static void _editor_init_syntax(editor_t* editor, char* name, char* path_pattern, syntax_def_t* defs);
static void _editor_init_cli_args(editor_t* editor, int argc, char** argv);
static void _editor_init_bviews(editor_t* editor, int argc, char** argv);

// Init editor from args
int editor_init(editor_t* editor, int argc, char** argv) {
    // Set editor defaults
    editor->tab_size = MLE_DEFAULT_TAB_SIZE;
    editor->tab_to_space = MLE_DEFAULT_TAB_TO_SPACE;
    editor_set_macro_toggle_key(editor, MLE_DEFAULT_MACRO_TOGGLE_KEY);
    _editor_init_kmaps(editor);
    _editor_init_syntaxes(editor);

    // Parse cli args
    _editor_init_cli_args(editor, argc, argv);

    // Init bviews
    _editor_init_bviews(editor, argc, argv);

    return MLE_OK;
}

// Run editor
int editor_run(editor_t* editor) {
    loop_context_t loop_ctx;
    loop_ctx.should_exit = 0;
    _editor_resize(editor);
    _editor_loop(editor, &loop_ctx);
    return MLE_OK;
}

// Deinit editor
int editor_deinit(editor_t* editor) {
    // TODO free stuff
    return MLE_OK;
}

// Prompt user for input
int editor_prompt(editor_t* editor, char* key, char* label, char** optret_answer) {
    loop_context_t loop_ctx;
    bview_t* prompt;

    // Init loop_ctx
    loop_ctx.invoker = editor->active;
    loop_ctx.should_exit = 0;
    loop_ctx.prompt_answer = NULL;

    // Init prompt
    editor_open_bview(editor, NULL, 0, 1, NULL, &prompt);
    prompt->type = MLE_BVIEW_TYPE_PROMPT;
    prompt->prompt_key = key;
    prompt->prompt_label = label;
    bview_push_kmap(prompt, editor->kmap_prompt);

    // Loop inside prompt
    _editor_loop(editor, &loop_ctx);

    // Set answer
    if (optret_answer) {
        *optret_answer = loop_ctx.prompt_answer;
    }

    // Restore previous focus
    editor_set_active(editor, loop_ctx.invoker);
    editor_close_bview(editor, prompt);

    return MLE_OK;
}

// Open a bview
int editor_open_bview(editor_t* editor, char* opt_path, size_t opt_path_len, int make_active, buffer_t* opt_buffer, bview_t** optret_bview) {
    bview_t* bview;
    int rc;
    bview = bview_new(editor, opt_path, opt_path_len, opt_buffer);
    DL_APPEND(editor->bviews, bview);
    if (make_active) {
        editor_set_active(editor, bview);
    }
    if (optret_bview) {
        *optret_bview = bview;
    }
    return MLE_OK;
}

// Close a bview
int editor_close_bview(editor_t* editor, bview_t* bview) {
    bview_t* prev;
    prev = bview->prev;
    if (!editor_bview_exists(editor, bview)) {
        MLE_RETURN_ERR("No bview %p in editor->bviews\n", bview);
    }
    DL_DELETE(editor->bviews, bview);
    bview_destroy(bview);
    if (prev) {
        editor_set_active(editor, prev);
    } else {
        editor_open_bview(editor, NULL, 0, 1, NULL, NULL);
    }
    return MLE_OK;
}

// Set the active bview
int editor_set_active(editor_t* editor, bview_t* bview) {
    if (!editor_bview_exists(editor, bview)) {
        MLE_RETURN_ERR("No bview %p in editor->bviews\n", bview);
    }
    editor->active = bview;
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
        if (tmp == bview) {
            return 1;
        }
    }
    return 0;
}

// Run editor loop
static void _editor_loop(editor_t* editor, loop_context_t* loop_ctx) {
    cmd_context_t cmd_ctx;
    cmd_function_t cmd_fn;

    // Init cmd_context
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
            cmd_fn(&cmd_ctx);
        } else {
            // TODO log error
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
static void _editor_resize(editor_t* editor) {
    bview_t* bview;
    bview_rect_t* bounds;

    editor->w = tb_width();
    editor->h = tb_height();

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
            bounds = &editor->rect_edit;
        }
        bview_resize(bview, bounds->x, bounds->y, bounds->w, bounds->h);
    }
}

// Display the editor
static void _editor_display(editor_t* editor) {
    // TODO
    tb_clear();
    bview_draw(editor->edit);
    bview_draw(editor->status);
    if (editor->popup) bview_draw(editor->popup);
    if (editor->prompt) bview_draw(editor->prompt);
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
    // TODO tb_poll_event
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
static cmd_function_t _editor_get_command(editor_t* editor, kinput_t input) {
    // TODO
    return NULL;
}

// Return a kinput_t given a key name
static int _editor_key_to_input(char* key, kinput_t* ret_input) {
    // TODO
    return MLE_OK;
}

// Init built-in kmaps
static void _editor_init_kmaps(editor_t* editor) {
    _editor_init_kmap(&editor->kmap_normal, "normal", cmd_insert_data, 0, (kmap_def_t[]){
        { cmd_insert_tab, "tab" },
        { cmd_delete_before, "backspace" },
        { cmd_delete_after, "delete" },
        { cmd_move_bol, "C-a" },
        { cmd_move_bol, "home" },
        { cmd_move_eol, "C-e" },
        { cmd_move_eol, "end" },
        { cmd_move_beginning, "M-\\" },
        { cmd_move_end, "M-/" },
        { cmd_move_left, "left" },
        { cmd_move_right, "right" },
        { cmd_move_up, "up" },
        { cmd_move_down, "down" },
        { cmd_move_page_up, "page-up" },
        { cmd_move_page_up, "page-down" },
        { cmd_move_to_line, "M-g" },
        { cmd_move_word_forward, "M-f" },
        { cmd_move_word_back, "M-b" },
        { cmd_anchor_sel_bound, "M-a" },
        { cmd_search, "C-f" },
        { cmd_search_next, "M-f" },
        { cmd_replace, "M-r" },
        { cmd_isearch, "C-r" },
        { cmd_delete_word_before, "C-w" },
        { cmd_delete_word_after, "M-d" },
        { cmd_cut, "C-k" },
        { cmd_uncut, "C-u" },
        { cmd_save, "C-o" },
        { cmd_open, "C-e" },
        { cmd_quit, "C-q" },
        { NULL, "" }
    });
    // TODO
}

// Init a single kmap
static void _editor_init_kmap(kmap_t** ret_kmap, char* name, cmd_function_t default_func, int allow_fallthru, kmap_def_t* defs) {
    kmap_t* kmap;
    kbinding_t* binding;

    kmap = calloc(1, sizeof(kmap_t));
    snprintf(kmap->name, MLE_KMAP_NAME_MAX_LEN, "%s", name);
    kmap->allow_fallthru = allow_fallthru;
    kmap->default_func = default_func;

    while (defs && defs->func) {
        binding = calloc(1, sizeof(kbinding_t));
        binding->func = defs->func;
        if (_editor_key_to_input(defs->key, &binding->input) != MLE_OK) {
            // TODO log error
            free(binding);
            defs++;
            continue;
        }
        HASH_ADD(hh, kmap->bindings, input, sizeof(kinput_t), binding);
        defs++;
    }

    *ret_kmap = kmap;
}

// Init built-in syntax map
static void _editor_init_syntaxes(editor_t* editor) {
    _editor_init_syntax(editor, "generic", "\\.(c|cpp|h|hpp|php|py|rb|sh|pl|go)$", (syntax_def_t[]){
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
        { "\\b\\$[a-zA-Z_0-9$]+\\b", NULL, TB_GREEN, TB_DEFAULT },
        { "\\b[A-Z_][A-Z0-9_]*\\b", NULL, TB_RED | TB_BOLD, TB_DEFAULT },
        { "\\b(-?(0x)?[0-9]+|true|false|null)\\b", NULL, TB_BLUE | TB_BOLD, TB_DEFAULT },
        { "[(){}<>.,;:?!+=/%\\[\\]$*-]", NULL, TB_RED | TB_BOLD, TB_DEFAULT },
        { "'([^']|\\')*'", NULL, TB_YELLOW | TB_BOLD, TB_DEFAULT },
        { "\"([^\"]|\\\")*\"", NULL, TB_YELLOW | TB_BOLD, TB_DEFAULT },
        { "/([^/]|/)*/", NULL, TB_YELLOW, TB_DEFAULT },
        { "/" "/.*$", NULL, TB_CYAN, TB_DEFAULT },
        { "/\\*", "\\*/", TB_CYAN, TB_DEFAULT },
        { "<\\?(php)?|\\?>", NULL, TB_GREEN, TB_DEFAULT },
        { "\\?>", "<\\?(php)?", TB_WHITE, TB_DEFAULT },
        { "\"\"\"", "\"\"\"", TB_YELLOW | TB_BOLD, TB_DEFAULT },
        { "\\s+$", NULL, TB_DEFAULT, TB_GREEN },
        { NULL, NULL, 0, 0 }
    });
}

// Init a single syntax
static void _editor_init_syntax(editor_t* editor, char* name, char* path_pattern, syntax_def_t* defs) {
    syntax_t* syntax;
    srule_node_t* node;

    syntax = calloc(1, sizeof(syntax_t));
    snprintf(syntax->name, MLE_SYNTAX_NAME_MAX_LEN, "%s", name);
    syntax->path_pattern = path_pattern;

    while (defs && defs->re) {
        node = calloc(1, sizeof(srule_node_t));
        if (defs->re_end) {
            node->srule = srule_new_multi(defs->re, strlen(defs->re), defs->re_end, strlen(defs->re_end), defs->fg, defs->bg);
        } else {
            node->srule = srule_new_single(defs->re, strlen(defs->re), defs->fg, defs->bg);
        }
        DL_APPEND(syntax->srules, node);
        defs++;
    }

    HASH_ADD_STR(editor->syntax_map, name, syntax);
}

// Parse cli args
static void _editor_init_cli_args(editor_t* editor, int argc, char** argv) {
    int c;
    while ((c = getopt(argc, argv, "hAms:t:v")) != -1) {
        switch (c) {
            case 'h':
                printf("mle version %s\n\n", MLE_VERSION);
                printf("Usage: mle [options] [file]...\n\n");
                printf("    -A           Allow tabs (disable tab-to-space)\n");
                printf("    -h           Show this message\n");
                printf("    -m <key>     Set macro toggle key (default: C-x)");
                printf("    -s <syntax>  Specify override syntax\n");
                printf("    -t <size>    Set tab size (default: %d)\n", MLE_DEFAULT_TAB_SIZE);
                printf("    -v           Print version and exit\n");
                printf("    file         At start up, open file\n");
                printf("    file:line    At start up, open file at line\n");
                exit(EXIT_SUCCESS);
            case 'A':
                editor->tab_to_space = 1;
                break;
            case 'm':
                if (editor_set_macro_toggle_key(editor, optarg) != MLE_OK) {
                    MLE_LOG_ERR("Could not set macro key to %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            case 's':
                editor->syntax_override = optarg;
                break;
            case 't':
                editor->tab_size = atoi(optarg);
                break;
            case 'v':
                printf("mle version %s\n", MLE_VERSION);
                exit(EXIT_SUCCESS);
        }
    }
}

// Init bviews
static void _editor_init_bviews(editor_t* editor, int argc, char** argv) {
    int i;
    char* colon;
    size_t linenum;
    bview_t* bview;
    char *path;
    size_t path_len;

    // Open bviews
    if (optind >= argc) {
        // Open blank
        editor_open_bview(editor, NULL, 0, 1, NULL, NULL);
    } else {
        // Open files
        for (i = optind; i < argc; i++) {
            path = argv[i];
            path_len = strlen(argv[i]);
            if (util_file_exists(path, path_len)) {
                editor_open_bview(editor, path, path_len, 1, NULL, NULL);
            } else if ((colon = strstr(path, ":")) != NULL) {
                path_len -= (size_t)(colon - path);
                linenum = strtoul(colon + 1, NULL, 10);
                if (linenum > 0) {
                    linenum -= 1;
                }
                editor_open_bview(editor, path, path_len, 1, NULL, &bview);
                MLE_CURSOR_APPLY_MARK_FN(bview->active_cursor, mark_move_to, linenum, 0);
            } else {
                editor_open_bview(editor, path, path_len, 1, NULL, NULL);
            }
        }
    }
}
