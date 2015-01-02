#ifndef __MLE_H
#define __MLE_H

#include <stdint.h>
#include <termbox.h>
#include "uthash.h"
#include "mlbuf.h"

// Typedefs
typedef struct editor_s editor_t; // A container for editor-wide globals
typedef struct bview_s bview_t; // A view of a buffer
typedef struct bview_rect_s bview_rect_t; // A rectangle in bview with a default styling
typedef struct cursor_s cursor_t; // A cursor (insertion mark + selection bound mark) in a buffer
typedef struct loop_context_s loop_context_t; // Context for a single _editor_loop
typedef struct cmd_context_s cmd_context_t; // Context for a single command invocation
typedef int (*cmd_function_t)(cmd_context_t* ctx); // A command function
typedef struct kinput_s kinput_t; // A single key input (similar to a tb_event from termbox)
typedef struct kmacro_s kmacro_t; // A sequence of kinputs and a name
typedef struct kmap_s kmap_t; // A map of keychords to functions
typedef struct kmap_node_s kmap_node_t; // A node in a list of keymaps
typedef struct kmap_def_s kmap_def_t; // A definition of a keymap
typedef struct kbinding_s kbinding_t; // A single binding in a keymap
typedef struct syntax_s syntax_t; // A syntax definition
typedef struct syntax_node_s syntax_node_t; // A node in a linked list of syntaxes
typedef struct syntax_def_s syntax_def_t; // A definition of a syntax
typedef struct srule_node_s srule_node_t; // A node in a linked list of srules
typedef struct tb_event tb_event_t; // A termbox event

// kinput_t
struct kinput_s {
    uint8_t mod;
    uint32_t ch;
    uint16_t key;
};

// bview_rect_t
struct bview_rect_s {
    int x;
    int y;
    int w;
    int h;
    uint16_t fg;
    uint16_t bg;
};

// editor_t
struct editor_s {
    int w;
    int h;
    bview_t* bviews;
    bview_t* popup;
    bview_t* active;
    bview_rect_t rect_status;
    bview_rect_t rect_prompt;
    bview_rect_t rect_popup;
    syntax_t* syntax_map;
    char* status;
    size_t status_len;
    int is_display_disabled;
    kmacro_t* macro_map;
    kinput_t macro_toggle_key;
    kmacro_t* macro_record;
    kmacro_t* macro_apply;
    size_t macro_apply_input_index;
    int is_recording_macro;
    kmap_t* kmap_normal;
    kmap_t* kmap_less;
    kmap_t* kmap_prompt;
    kmap_t* kmap_save;
    kmap_t* kmap_quit;
    kmap_t* kmap_isearch;
    kmap_t* kmap_fsearch;
    int tab_size;
    int tab_to_space;
    char* syntax_override;
};

// syntax_def_t
struct syntax_def_s {
    char* re;
    char* re_end;
    uint16_t fg;
    uint16_t bg;
};

// syntax_node_t
struct syntax_node_s {
    srule_t* srule;
    syntax_node_t* next;
    syntax_node_t* prev;
};

// syntax_t
struct syntax_s {
#define MLE_SYNTAX_NAME_MAX_LEN 16
    char name[MLE_SYNTAX_NAME_MAX_LEN + 1];
    char* path_pattern;
    srule_node_t* srules;
    UT_hash_handle hh;
};

// bview_t
struct bview_s {
    int x;
    int y;
    int w;
    int h;
    int is_chromeless;
    int line_num_width;
    bview_rect_t rect_caption;
    bview_rect_t rect_lines;
    bview_rect_t rect_margin_left;
    bview_rect_t rect_buffer;
    bview_rect_t rect_margin_right;
    buffer_t* buffer;
    int viewport_x;
    int viewport_y;
    int viewport_scope_x;
    int viewport_scope_y;
    bview_t* split_parent;
    bview_t* split_child;
    float split_factor;
    int split_is_vertical;
    int is_prompt;
    char* prompt_key;
    char* prompt_label;
    kmap_node_t* kmap_stack;
    kmap_node_t* kmap_tail;
    cursor_t* cursors;
    cursor_t* active_cursor;
    char* path;
    size_t path_len;
    int tab_size;
    int tab_to_space;
    bview_t* next;
    bview_t* prev;
};

// cursor_t
struct cursor_s {
    bview_t* bview;
    mark_t* mark;
    mark_t* sel_bound;
    int is_sel_bound_anchored;
    int is_asleep;
    cursor_t* next;
    cursor_t* prev;
};

// kmacro_t
struct kmacro_s {
    kinput_t key;
    kinput_t* inputs;
    size_t inputs_len;
    size_t inputs_cap;
    UT_hash_handle hh;
};

// kbinding_t
struct kbinding_s {
    kinput_t input;
    cmd_function_t func;
    UT_hash_handle hh;
};

// kmap_node_t
struct kmap_node_s {
    kmap_t* kmap;
    bview_t* src;
    kmap_node_t* next;
    kmap_node_t* prev;
};

// kmap_def_t
struct kmap_def_s {
    cmd_function_t func;
    char* key;
};

// kmap_t
struct kmap_s {
#define MLE_KMAP_NAME_MAX_LEN 16
    char name[MLE_KMAP_NAME_MAX_LEN + 1];
    kbinding_t* bindings;
    int allow_fallthru;
    cmd_function_t default_func;
};

// cmd_context_t
struct cmd_context_s {
    editor_t* editor;
    cursor_t* cursor;
    kinput_t input;
    loop_context_t* loop_ctx;
};

// loop_context_t
struct loop_context_s {
    int is_prompt;
    int should_exit;
    char* prompt_answer;
};

// editor functions
int editor_init(editor_t* editor, int argc, char** argv);
int editor_run(editor_t* editor);
int editor_deinit(editor_t* editor);
int editor_prompt(editor_t* editor, char* key, char* label, char** optret_answer);
int editor_open_bview(editor_t* editor, char* path, size_t path_len, int make_active, bview_t* opt_before, bview_t** optret_bview);
int editor_close_bview(editor_t* editor, bview_t* bview);
int editor_set_active_bview(editor_t* editor, bview_t* bview);
int editor_set_macro_toggle_key(editor_t* editor, char* key);

// cmd functions
int cmd_insert_data(cmd_context_t* ctx);
int cmd_insert_tab(cmd_context_t* ctx);
int cmd_delete_before(cmd_context_t* ctx);
int cmd_delete_after(cmd_context_t* ctx);
int cmd_move_bol(cmd_context_t* ctx);
int cmd_move_eol(cmd_context_t* ctx);
int cmd_move_beginning(cmd_context_t* ctx);
int cmd_move_end(cmd_context_t* ctx);
int cmd_move_left(cmd_context_t* ctx);
int cmd_move_right(cmd_context_t* ctx);
int cmd_move_up(cmd_context_t* ctx);
int cmd_move_down(cmd_context_t* ctx);
int cmd_move_page_up(cmd_context_t* ctx);
int cmd_move_page_down(cmd_context_t* ctx);
int cmd_move_to_line(cmd_context_t* ctx);
int cmd_move_word_forward(cmd_context_t* ctx);
int cmd_move_word_back(cmd_context_t* ctx);
int cmd_anchor_sel_bound(cmd_context_t* ctx);
int cmd_search(cmd_context_t* ctx);
int cmd_search_next(cmd_context_t* ctx);
int cmd_replace(cmd_context_t* ctx);
int cmd_isearch(cmd_context_t* ctx);
int cmd_delete_word_before(cmd_context_t* ctx);
int cmd_delete_word_after(cmd_context_t* ctx);
int cmd_cut(cmd_context_t* ctx);
int cmd_uncut(cmd_context_t* ctx);
int cmd_save(cmd_context_t* ctx);
int cmd_open(cmd_context_t* ctx);
int cmd_quit(cmd_context_t* ctx);

// bview functions
int bview_push_kmap(bview_t* bview, kmap_t* kmap);

// util functions
int util_file_exists(char* path, size_t path_len);


// Macros
#define MLE_VERSION "0.1"

#define MLE_RC_OK 0
#define MLE_RC_ERR 1

#define MLE_DEFAULT_TAB_SIZE 4
#define MLE_DEFAULT_TAB_TO_SPACE 1
#define MLE_DEFAULT_MACRO_TOGGLE_KEY "C-x"

#define MLE_CURSOR_APPLY_MARK_FN(cursor, mark_fn, ...)

/*
Features
[ ] tab stops
[ ] tab to space
[ ] file:line#(s) to open
[ ] char display width (for tabs)
[ ] code folding
[ ] window splits
[ ] run selection thru cmd
[ ] macros
[ ] multiple cursors
[ ] command prompt
[ ] named marks
[ ] named registers
[ ] modes (push/pop kmap on kmap_stack)
[ ] cli options
[ ] rc file


mode:
- name
- bindings
- fallthrough?
- default fn?

mode instance:
- mode
- s


*/

#endif
