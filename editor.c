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

// Init editor from args
int editor_init(editor_t* editor, int argc, char** argv) {
    int c;
    int i;
    char* colon;
    size_t linenum;
    bview_t* bview;
    char *path;
    size_t path_len;

    // Set editor defaults
    editor->tab_size = MLE_DEFAULT_TAB_SIZE;
    editor->tab_to_space = MLE_DEFAULT_TAB_TO_SPACE;
    editor_set_macro_toggle_key(editor, MLE_DEFAULT_MACRO_TOGGLE_KEY);
    _editor_init_kmaps(editor);
    _editor_init_syntaxes(editor);

    // Parse cli args
    while ((c = getopt(argc, argv, "hAms:t:v")) != -1) {
        switch (c) {
            case 'h':
                printf("mle version %s\n\n", MLE_VERSION);
                printf("Usage: mle [options] [file]...\n\n");
                printf("    -A           Disable tab-to-space (default: enabled)\n");
                printf("    -h           Show this message\n");
                printf("    -m <key>     Set macro toggle key (default: C-x)");
                printf("    -s <syntax>  Specify override syntax\n");
                printf("    -t <size>    Set tab size (default: %d)\n", MLE_DEFAULT_TAB_SIZE);
                printf("    -v           Print version and exit\n");
                printf("    file         Open file at start up\n");
                printf("    file:line    Open file at line at start up\n");
                exit(EXIT_SUCCESS);
            case 'A':
                editor->tab_to_space = 1;
                break;
            case 'm':
                if (editor_set_macro_toggle_key(editor, optarg) != MLE_RC_OK) {
                    fprintf(stderr, "Could not set macro key to %s\n", optarg);
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

    return MLE_RC_OK;
}

// Run editor
int editor_run(editor_t* editor) {
    loop_context_t loop_ctx;
    loop_ctx.is_prompt = 0;
    loop_ctx.should_exit = 0;
    _editor_resize(editor);
    _editor_loop(editor, &loop_ctx);
    return MLE_RC_OK;
}


// Deinit editor
int editor_deinit(editor_t* editor) {
    // TODO free stuff
    return MLE_RC_OK;
}

// Prompt user for input
int editor_prompt(editor_t* editor, char* key, char* label, char** optret_answer) {
    bview_t* invoker;
    bview_t* prompt;
    loop_context_t loop_ctx;

    // Init loop_ctx
    loop_ctx.is_prompt = 1;
    loop_ctx.should_exit = 0;
    loop_ctx.prompt_answer = NULL;

    // Init prompt
    invoker = editor->active;
    editor_open_bview(editor, NULL, 0, 1, NULL, &prompt);
    prompt->is_prompt = 1;
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
    editor_set_active_bview(editor, invoker);
    editor_close_bview(editor, prompt);

    return MLE_RC_OK;
}

// Open a bview
int editor_open_bview(editor_t* editor, char* path, size_t path_len, int make_active, bview_t* opt_before, bview_t** optret_bview) {
    // TODO
    return MLE_RC_OK;
}

// Close a bview
int editor_close_bview(editor_t* editor, bview_t* bview) {
    // TODO
    return MLE_RC_OK;
}

// Set the active bview
int editor_set_active_bview(editor_t* editor, bview_t* bview) {
    // TODO
    return MLE_RC_OK;
}

// Set macro toggle key
int editor_set_macro_toggle_key(editor_t* editor, char* key) {
    return _editor_key_to_input(key, &editor->macro_toggle_key);
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
    // TODO
}

// Display the editor
static void _editor_display(editor_t* editor) {
    // TODO
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
    return MLE_RC_OK;
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
        { cmd_move_page_up, "page_up" },
        { cmd_move_page_up, "page_down" },
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
        if (_editor_key_to_input(defs->key, &binding->input) != MLE_RC_OK) {
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
    _editor_init_syntax(editor, "php", "\\.php$", (syntax_def_t[]){
        { "\\$[a-zA-Z_0-9$]*|[=!<>-]", NULL, TB_GREEN, TB_DEFAULT },
        { "\\b(class|var|const|static|final|private|public|protected|function|"
          "switch|case|default|endswitch|if|else|elseif|endif|for|foreach|"
          "endfor|endforeach|while|endwhile|new|die|exit|echo|continue|break|"
          "include|require|include_once|require_once|return|abstract|"
          "interface)\\b", NULL, TB_YELLOW, TB_DEFAULT },
        { "[(){}.,;:?!+=/%-\\[\\]]", NULL, TB_RED | TB_BOLD, TB_DEFAULT },
        { "\\b(-?[0-9]+|true|false|null)\\b", NULL, TB_BLUE | TB_BOLD, TB_DEFAULT },
        { "(\\$|=>|->|::)", NULL, TB_GREEN | TB_BOLD, TB_DEFAULT },
        { "'([^']|\\')*'", NULL, TB_YELLOW | TB_BOLD, TB_DEFAULT },
        { "\"([^\"]|\\\")*\"", NULL, TB_YELLOW | TB_BOLD, TB_DEFAULT },
        { "//.*$", NULL, TB_CYAN, TB_DEFAULT },
        { "/\\*", "\\*/", TB_CYAN, TB_DEFAULT },
        { "<\\?(php)?|\\?>", NULL, TB_GREEN, TB_DEFAULT },
        { "\\?>", "<\\?(php)?", TB_WHITE, TB_DEFAULT },
        { "\\s+$", NULL, TB_DEFAULT, TB_GREEN },
        { NULL, NULL, 0, 0 }
    });
    // TODO
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
    }

    HASH_ADD_STR(editor->syntax_map, name, syntax);
}
