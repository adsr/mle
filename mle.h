#ifndef __MLE_H
#define __MLE_H

#include <stdint.h>
#include <termbox.h>
#include "uthash.h"
#include "mlbuf/mlbuf.h"

// Typedefs
typedef struct editor_s editor_t; // A container for editor-wide globals
typedef struct init_file_s init_file_t; // A list of files to open at init
typedef struct bview_s bview_t; // A view of a buffer
typedef struct bview_rect_s bview_rect_t; // A rectangle in bview with a default styling
typedef struct cursor_s cursor_t; // A cursor (insertion mark + selection bound mark) in a buffer
typedef struct cmd_context_s cmd_context_t; // Context for a single command invocation
typedef int (*cmd_function_t)(cmd_context_t* ctx); // A command function
typedef struct stack_val_s stack_val_t; // A value on the value stack
typedef struct kevent_s kevent_t; // A single key input (similar to a tb_event from termbox)
typedef struct kmacro_s kmacro_t; // A sequence of kevents and a name
typedef struct kmap_s kmap_t; // A map of keychords to functions
typedef struct kmap_node_s kmap_node_t; // A node in a list of keymaps
typedef struct kbinding_s kbinding_t; // A single binding in a keymap
typedef struct kqueue_s kqueue_t; // A node in a queue of keys
typedef struct srule_map_s srule_map_t; // A map of srules
typedef struct tb_event tb_event_t; // A termbox event

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
    buffer_t* buffers;
    bview_t* bviews;
    bview_t* edit;
    bview_rect_t rect_status;
    bview_t* prompt;
    bview_t* active;
    kmap_t* default_kmap;
    kqueue_t* kqueue;
    kqueue_t* kqueue_tail;
    stack_val_t* stack_vals;
    kmacro_t* kmacros;
    init_file_t* init_files;
    srule_map_t* srule_map;
    int is_recording_macro;
    char* status;
    size_t status_len;
    int is_render_disabled;
    int should_exit;
};

// init_file_t
struct init_file_s {
    char* path;
    size_t line_num;
    init_file_t* next;
    init_file_t* prev;
};

// srule_map_t
struct srule_map_t {
#define MLE_SRULE_MAP_NAME_MAX_LEN 16
    char name[MLE_SRULE_MAP_NAME_MAX_LEN + 1];
    srule_t* srule;
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
    kmap_node_t* kmap_stack;
    kmap_node_t* kmap_tail;
    cursor_t* cursors;
    cursor_t* active_cursor;
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

// kevent_t
struct kevent_s {
    uint8_t mod;
    uint32_t ch;
    uint16_t key;
};

// kmacro_t
struct kmacro_s {
#define MLE_KMACRO_NAME_MAX_LEN 16
    char name[MLE_KMACRO_NAME_MAX_LEN + 1];
    kevent_t* event;
    size_t event_len;
    size_t event_cap;
    UT_hash_handle hh;
};

// kbinding_t
struct kbinding_s {
    kevent_t input;
    cmd_function_t func;
    UT_hash_handle hh;
};

// kmap_node_t
struct kmap_node_s {
    kmap_t* kmap;
    kmap_node_t* next;
    kmap_node_t* prev;
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
    kevent_t event;
    tb_event_t raw_event;
};

// stack_val_t
struct stack_val_s {
#define MLE_STACK_VAL_TYPE_INT 0
#define MLE_STACK_VAL_TYPE_FLOAT 1
#define MLE_STACK_VAL_TYPE_STRING 2
    int type;
    union {
        uint64_t i;
        double f;
        char* s;
    };
    stack_val_t* next;
    stack_val_t* prev;
};

// editor functions
int editor_init(editor_t* editor);
int editor_loop(editor_t* editor);
int editor_deinit(editor_t* editor);

// Macros

#define MLE_RC_OK 0
#define MLE_RC_ERR 1

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
*/

#endif
