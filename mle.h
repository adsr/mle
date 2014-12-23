#ifndef __MLE_H
#define __MLE_H

// Typedefs
typedef struct editor_s editor_t; // A container for editor-wide globals
typedef struct bview_s bview_t; // A view of a buffer
typedef struct bview_rect_s bview_rect_t; // A rectangle in bview with a default styling
typedef struct cursor_s cursor_t; // A cursor (insertion mark + selection bound mark) in a buffer
typedef struct kevent_s kevent_t; // A single key input (similar to a tb_event from termbox)
typedef struct kmacro_s kmacro_t; // A sequence of kevents and a name
typedef struct kevent_node_s kevent_node_t; // A node in a list of kevents
typedef struct kmap_s kmap_t; // A map of keychords to functions
typedef struct kmap_node_s kmap_node_t; // A node in a list of keymaps
typedef struct kbinding_s kbinding_t; // A single binding in a keymap
typedef struct kqueue_s kqueue_t; // A node in a queue of keys
typedef struct stack_val_s stack_val_t; // A value on the value stack
typedef struct cmd_context_s cmd_context_t; // Context for a single command invocation
typedef int (*cmd_function_t)(cmd_context_t* ctx); // A command function

// bview_t
struct bview_rect_s {
    int x;
    int y;
    int w;
    int h;
    uint16_t fg;
    uint16_t bg;
};
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
    kmap_node_t* kmaps;
    kmap_node_t* kmaps_tail;
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

// cmd_context_t
struct cmd_context_s {
    editor_t* editor;
    cursor_t* cursor;
    kevent_t event;
    tb_event raw_event;
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

// stack_val_t
struct stack_val_s {
    int type;
    size_t slen;
    union {
        uint64_t i;
        double f;
        char* s;
    }
    stack_val_t* next;
    stack_val_t* prev;
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
    int is_recording_macro;
    char* status;
    size_t status_len;
    int is_render_disabled;
    int should_exit;
};

/*
use atto as template

main loop:
    get input
    get command from current keymap
    exec command
    redraw

keymapstack:
    linked list of hashes:
        hash key=(mod|key|ch) val=commandfn

modes:
    push/pop keymap on keystack

features:
    tab stops
    char display width (tab)
    code folding
    window splits
    replace selection with stdout of cmd
    macros
    multiple cursors
    command prompt
    marks
    registers/clipboards
    modes

cmd line switches (rc file is just cmd line switches in file):
    tab stop
    tab to space
    file to open
    --map default:type
    --map CW:delete_word_back
    --map Mg:goto_line
    --macro derp 'Mg 3 0 0 enter'
    --syn php.pattern:'.php'
    --syn php.single:'(function|derp|derp)'
    --syn php.multi:'/*':'*/'

*/

#endif
