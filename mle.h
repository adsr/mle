#ifndef __MLE_H
#define __MLE_H

#include <stdint.h>
#include <limits.h>
#include <uthash.h>
#include <lua5.4/lua.h>
#include <lua5.4/lualib.h>
#include <lua5.4/lauxlib.h>
#include "termbox2.h"
#include "mlbuf.h"

// Typedefs
typedef struct editor_s editor_t; // A container for editor-wide globals
typedef struct bview_s bview_t; // A view of a buffer
typedef struct bview_rect_s bview_rect_t; // A rectangle in bview with a default styling
typedef struct bview_listener_s bview_listener_t; // A listener to buffer events in a bview
typedef void (*bview_listener_cb_t)(bview_t *bview, baction_t *action, void *udata); // A bview_listener_t callback
typedef struct cursor_s cursor_t; // A cursor (insertion mark + anchor mark) in a buffer
typedef struct loop_context_s loop_context_t; // Context for a single _editor_loop
typedef struct cmd_s cmd_t; // A command definition
typedef struct cmd_context_s cmd_context_t; // Context for a single command invocation
typedef struct observer_s observer_t; // An observer of a cmd or event
typedef struct kinput_s kinput_t; // A single key input (similar to a tb_event from termbox)
typedef struct kmacro_s kmacro_t; // A sequence of kinputs and a name
typedef struct kmap_s kmap_t; // A map of keychords to functions
typedef struct kmap_node_s kmap_node_t; // A node in a list of keymaps
typedef struct kbinding_def_s kbinding_def_t; // A definition of a keymap
typedef struct kbinding_s kbinding_t; // A single binding in a keymap
typedef struct syntax_s syntax_t; // A syntax definition
typedef struct syntax_node_s syntax_node_t; // A node in a linked list of syntaxes
typedef struct srule_def_s srule_def_t; // A definition of a syntax
typedef struct aproc_s aproc_t; // An asynchronous process
typedef void (*aproc_cb_t)(aproc_t *self, char *buf, size_t buf_len); // An aproc_t callback
typedef struct editor_prompt_params_s editor_prompt_params_t; // Extra params for editor_prompt
typedef struct tb_event tb_event_t; // A termbox event
typedef struct prompt_history_s prompt_history_t; // A map of prompt histories keyed by prompt_str
typedef struct prompt_hnode_s prompt_hnode_t; // A node in a linked list of prompt history
typedef int (*cmd_func_t)(cmd_context_t *ctx); // A command function
typedef int (*observer_func_t)(char *event_name, void *event_data, void *udata); // An event callback function
typedef struct uscript_s uscript_t; // A userscript
typedef struct uhandle_s uhandle_t; // A method handle in a uscript

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
    bview_t *top_bviews;
    bview_t *all_bviews;
    bview_t *active;
    bview_t *active_edit;
    bview_t *active_edit_last;
    bview_t *active_edit_root;
    bview_t *status;
    bview_t *prompt;
    bview_rect_t rect_edit;
    bview_rect_t rect_status;
    bview_rect_t rect_prompt;
    syntax_t *syntax_map;
    syntax_t *syntax_last;
    int is_display_disabled;
    kmacro_t *macro_map;
    kinput_t macro_toggle_key;
    kmacro_t *macro_record;
    kmacro_t *macro_apply;
    size_t macro_apply_input_index;
    int is_recording_macro;
    char *startup_macro_name;
    cmd_t *cmd_map;
    kmap_t *kmap_map;
    kmap_t *kmap_normal;
    kmap_t *kmap_prompt_input;
    kmap_t *kmap_prompt_yn;
    kmap_t *kmap_prompt_yna;
    kmap_t *kmap_prompt_ok;
    kmap_t *kmap_prompt_isearch;
    kmap_t *kmap_menu;
    prompt_history_t *prompt_history;
    char *kmap_init_name;
    kmap_t *kmap_init;
    aproc_t *aprocs;
    uscript_t *uscripts;
    observer_t *observers;
    FILE *tty;
    int ttyfd;
    char *syntax_override;
    int linenum_type;
    int tab_width;
    int tab_to_space;
    int trim_paste;
    int auto_indent;
    int read_rc_file;
    int highlight_bracket_pairs;
    int color_col;
    int soft_wrap;
    int coarse_undo;
    int mouse_support;
    int viewport_scope_x; // TODO cli option
    int viewport_scope_y; // TODO cli option
    int headless_mode;
    loop_context_t *loop_ctx;
    int loop_depth;
    int is_in_init;
    int is_mousedown;
    char *insertbuf;
    size_t insertbuf_size;
    char *cut_buffer;
    int user_input_count;
    #define MLE_ERRSTR_SIZE 256
    char errstr[MLE_ERRSTR_SIZE];
    char infostr[MLE_ERRSTR_SIZE];
    int debug_exit_after_startup;
    int debug_dump_state_on_exit;
    int debug_key_input;
    #define MLE_DISPLAY_KEYS_NKEYS 8
    #define MLE_MAX_KEYNAME_LEN 16
    int debug_display_keys;
    char display_keys_buf[MLE_DISPLAY_KEYS_NKEYS * MLE_MAX_KEYNAME_LEN];
    int exit_code;
};

// srule_def_t
struct srule_def_s {
    char *re;
    char *re_end;
    uint16_t fg;
    uint16_t bg;
};

// syntax_node_t
struct syntax_node_s {
    srule_t *srule;
    syntax_node_t *next;
    syntax_node_t *prev;
};

// syntax_t
struct syntax_s {
    char *name;
    char *path_pattern;
    int tab_width;
    int tab_to_space;
    srule_node_t *srules;
    UT_hash_handle hh;
};

// bview_t
struct bview_s {
    #define MLE_BVIEW_TYPE_EDIT 0
    #define MLE_BVIEW_TYPE_STATUS 1
    #define MLE_BVIEW_TYPE_PROMPT 2
    editor_t *editor;
    int x;
    int y;
    int w;
    int h;
    int is_resized;
    int type;
    int linenum_width;
    int abs_linenum_width;
    int rel_linenum_width;
    bview_rect_t rect_caption;
    bview_rect_t rect_lines;
    bview_rect_t rect_margin_left;
    bview_rect_t rect_buffer;
    bview_rect_t rect_margin_right;
    buffer_t *buffer;
    bint_t viewport_x;
    bint_t viewport_x_vcol;
    bint_t viewport_y;
    mark_t *viewport_mark;
    int viewport_scope_x;
    int viewport_scope_y;
    bview_t *split_parent;
    bview_t *split_child;
    float split_factor;
    int split_is_vertical;
    char *prompt_str;
    char *path;
    bint_t startup_linenum;
    kmap_node_t *kmap_stack;
    kmap_node_t *kmap_tail;
    cursor_t *cursors;
    cursor_t *active_cursor;
    char *last_search;
    srule_t *isearch_rule;
    int tab_width;
    int tab_to_space;
    int soft_wrap;
    syntax_t *syntax;
    aproc_t *aproc;
    cmd_func_t menu_callback;
    int is_menu;
    #ifndef PATH_MAX
    #define PATH_MAX 4096
    #endif
    char browse_path[PATH_MAX + 1];
    int id;
    bview_listener_t *listeners;
    bview_t *top_next;
    bview_t *top_prev;
    bview_t *all_next;
    bview_t *all_prev;
};

// bview_listener_t
struct bview_listener_s {
    bview_listener_cb_t callback;
    void *udata;
    bview_listener_t *next;
    bview_listener_t *prev;
};

// cursor_t
struct cursor_s {
    bview_t *bview;
    mark_t *mark;
    mark_t *anchor;
    int is_anchored;
    int is_temp_anchored;
    int is_block;
    int is_asleep;
    srule_t *sel_rule;
    char *cut_buffer;
    cursor_t *next;
    cursor_t *prev;
};

// kmacro_t
struct kmacro_s {
    char *name;
    kinput_t *inputs;
    size_t inputs_len;
    size_t inputs_cap;
    UT_hash_handle hh;
};

// cmd_t
struct cmd_s {
    char *name;
    cmd_func_t func;
    void *udata;
    int is_resolved;
    UT_hash_handle hh;
};

// kbinding_def_t
struct kbinding_def_s {
    #define MLE_KBINDING_DEF(pcmdname, pkeypatt)             { (pcmdname), (pkeypatt), NULL }
    #define MLE_KBINDING_DEF_EX(pcmdname, pkeypatt, pstatp)  { (pcmdname), (pkeypatt), (pstatp) }
    char *cmd_name;
    char *key_patt;
    char *static_param;
};

// kbinding_t
struct kbinding_s {
    kinput_t input;
    char *cmd_name;
    cmd_t *cmd;
    char *static_param;
    char *key_patt;
    int is_leaf;
    kbinding_t *children;
    UT_hash_handle hh;
};

// kmap_node_t
struct kmap_node_s {
    kmap_t *kmap;
    bview_t *bview;
    kmap_node_t *next;
    kmap_node_t *prev;
};

// kmap_t
struct kmap_s {
    char *name;
    kbinding_t *bindings;
    int allow_fallthru;
    char *default_cmd_name;
    cmd_t *default_cmd;
    UT_hash_handle hh;
};

// cmd_context_t
struct cmd_context_s {
    #define MLE_PASTEBUF_INCR 1024
    #define MLE_MAX_NUMERIC_PARAMS 8
    #define MLE_MAX_WILDCARD_PARAMS 8
    editor_t *editor;
    loop_context_t *loop_ctx;
    cmd_t *cmd;
    buffer_t *buffer;
    bview_t *bview;
    cursor_t *cursor;
    kinput_t input;
    char *static_param;
    uintmax_t numeric_params[MLE_MAX_NUMERIC_PARAMS];
    int numeric_params_len;
    uint32_t wildcard_params[MLE_MAX_WILDCARD_PARAMS];
    int wildcard_params_len;
    int is_user_input;
    kinput_t *pastebuf;
    size_t pastebuf_len;
    size_t pastebuf_size;
    int has_pastebuf_leftover;
    kinput_t pastebuf_leftover;
};

// observer_t
struct observer_s {
    char *event_patt;
    observer_func_t callback;
    void *udata;
    observer_t *next;
    observer_t *prev;
};

// loop_context_t
struct loop_context_s {
    #define MLE_LOOP_CTX_MAX_NUMERIC_LEN 20
    #define MLE_LOOP_CTX_MAX_COMPLETE_TERM_SIZE 256
    bview_t *invoker;
    char numeric[MLE_LOOP_CTX_MAX_NUMERIC_LEN + 1];
    kbinding_t *numeric_node;
    int numeric_len;
    kbinding_t *binding_node;
    int need_more_input;
    int should_exit;
    char *prompt_answer;
    cmd_func_t prompt_callack;
    prompt_hnode_t *prompt_hnode;
    int tab_complete_index;
    char tab_complete_term[MLE_LOOP_CTX_MAX_COMPLETE_TERM_SIZE];
    cmd_context_t last_cmd_ctx;
    str_t last_insert;
    kinput_t *input_trail;
    size_t input_trail_len;
    size_t input_trail_cap;
    size_t input_trail_idx;
};

// aproc_t
struct aproc_s {
    editor_t *editor;
    void *owner;
    aproc_t **owner_aproc;
    FILE *rpipe;
    FILE *wpipe;
    pid_t pid;
    int rfd;
    int wfd;
    int is_done;
    aproc_cb_t callback;
    aproc_t *next;
    aproc_t *prev;
};

// editor_prompt_params_t
struct editor_prompt_params_s {
    char *data;
    int data_len;
    kmap_t *kmap;
    bview_listener_cb_t prompt_cb;
    void *prompt_cb_udata;
};

// prompt_history_t
struct prompt_history_s {
    char *prompt_str;
    prompt_hnode_t *prompt_hlist;
    UT_hash_handle hh;
};

// prompt_hnode_t
struct prompt_hnode_s {
    char *data;
    bint_t data_len;
    prompt_hnode_t *prev;
    prompt_hnode_t *next;
};

// uscript_t
struct uscript_s {
    editor_t *editor;
    lua_State *L;
    uhandle_t *uhandles;
    uscript_t *prev;
    uscript_t *next;
};

// uhandle_t
struct uhandle_s {
    uscript_t *uscript;
    int callback_ref;
    uhandle_t *next;
    uhandle_t *prev;
};

// editor functions
int editor_init(editor_t *editor, int argc, char **argv);
int editor_run(editor_t *editor);
int editor_deinit(editor_t *editor);
int editor_prompt(editor_t *editor, char *prompt, editor_prompt_params_t *params, char **optret_answer);
int editor_menu(editor_t *editor, cmd_func_t fn_callback, char *opt_buf_data, int opt_buf_data_len, aproc_t *opt_aproc, bview_t **optret_menu);
int editor_open_bview(editor_t *editor, bview_t *opt_parent, int type, char *opt_path, int opt_path_len, int make_active, bint_t linenum, int skip_resize, buffer_t *opt_buffer, bview_t **optret_bview);
int editor_close_bview(editor_t *editor, bview_t *bview, int *optret_num_closed);
int editor_set_active(editor_t *editor, bview_t *bview);
int editor_bview_edit_count(editor_t *editor);
int editor_count_bviews_by_buffer(editor_t *editor, buffer_t *buffer);
int editor_register_cmd(editor_t *editor, cmd_t *cmd);
int editor_register_observer(editor_t *editor, char *event_patt, void *udata, observer_func_t fn_callback, observer_t **optret_observer);
int editor_notify_observers(editor_t *editor, char *event_name, void *event_data);
int editor_destroy_observer(editor_t *editor, observer_t *observer);
int editor_get_input(editor_t *editor, loop_context_t *loop_ctx, cmd_context_t *ctx);
int editor_display(editor_t *editor);
int editor_debug_dump(editor_t *editor, FILE *fp);
int editor_force_redraw(editor_t *editor);
int editor_set_input_mode(editor_t *editor);

// bview functions
bview_t *bview_get_split_root(bview_t *self);
bview_t *bview_new(editor_t *editor, int type, char *opt_path, int opt_path_len, buffer_t *opt_buffer);
int bview_add_cursor_asleep(bview_t *self, bline_t *opt_bline, bint_t opt_col, cursor_t **optret_cursor);
int bview_add_cursor(bview_t *self, bline_t *opt_bline, bint_t opt_col, cursor_t **optret_cursor);
int bview_add_listener(bview_t *self, bview_listener_cb_t fn_callback, void *udata);
int bview_center_viewport_y(bview_t *self);
int bview_destroy(bview_t *self);
int bview_destroy_listener(bview_t *self, bview_listener_t *listener);
int bview_draw(bview_t *self);
int bview_draw_cursor(bview_t *self, int set_real_cursor);
int bview_get_active_cursor_count(bview_t *self);
int bview_get_screen_coords(bview_t *self, mark_t *mark, int *ret_x, int *ret_y, struct tb_cell **optret_cell);
int bview_max_viewport_y(bview_t *self);
int bview_open(bview_t *self, char *path, int path_len);
int bview_pop_kmap(bview_t *bview, kmap_t **optret_kmap);
int bview_push_kmap(bview_t *bview, kmap_t *kmap);
int bview_rectify_viewport(bview_t *self);
int bview_remove_cursor(bview_t *self, cursor_t *cursor);
int bview_remove_cursors_except(bview_t *self, cursor_t *one);
int bview_resize(bview_t *self, int x, int y, int w, int h);
int bview_screen_to_bline_col(bview_t *self, int x, int y, bview_t **ret_bview, bline_t **ret_bline, bint_t *ret_col);
int bview_set_syntax(bview_t *self, char *opt_syntax);
int bview_set_viewport_y(bview_t *self, bint_t y, int do_rectify);
int bview_split(bview_t *self, int is_vertical, float factor, bview_t **optret_bview);
int bview_wake_sleeping_cursors(bview_t *self);
int bview_zero_viewport_y(bview_t *self);

// cursor functions
int cursor_clone(cursor_t *cursor, int use_srules, cursor_t **ret_clone);
int cursor_cut_copy(cursor_t *cursor, int is_cut, int use_srules, int append);
int cursor_destroy(cursor_t *cursor);
int cursor_drop_anchor(cursor_t *cursor, int use_srules);
int cursor_get_lo_hi(cursor_t *cursor, mark_t **ret_lo, mark_t **ret_hi);
int cursor_get_mark(cursor_t *cursor, mark_t **ret_mark);
int cursor_get_anchor(cursor_t *cursor, mark_t **ret_anchor);
int cursor_lift_anchor(cursor_t *cursor);
int cursor_replace(cursor_t *cursor, int interactive, char *opt_regex, char *opt_replacement);
int cursor_select_between(cursor_t *cursor, mark_t *a, mark_t *b, int use_srules);
int cursor_select_by(cursor_t *cursor, const char *strat, int use_srules);
int cursor_select_by_bracket(cursor_t *cursor, int use_srules);
int cursor_select_by_string(cursor_t *cursor, int use_srules);
int cursor_select_by_word_back(cursor_t *cursor, int use_srules);
int cursor_select_by_word(cursor_t *cursor, int use_srules);
int cursor_select_by_word_forward(cursor_t *cursor, int use_srules);
int cursor_toggle_anchor(cursor_t *cursor, int use_srules);
int cursor_uncut(cursor_t *cursor);
int cursor_uncut_last(cursor_t *cursor);

// cmd functions
int cmd_align_cursors(cmd_context_t *ctx);
int cmd_anchor_by(cmd_context_t *ctx);
int cmd_apply_macro_by(cmd_context_t *ctx);
int cmd_apply_macro(cmd_context_t *ctx);
int cmd_blist(cmd_context_t *ctx);
int cmd_browse(cmd_context_t *ctx);
int cmd_close(cmd_context_t *ctx);
int cmd_copy_by(cmd_context_t *ctx);
int cmd_copy(cmd_context_t *ctx);
int cmd_ctag(cmd_context_t *ctx);
int cmd_cut_by(cmd_context_t *ctx);
int cmd_cut(cmd_context_t *ctx);
int cmd_delete_after(cmd_context_t *ctx);
int cmd_delete_before(cmd_context_t *ctx);
int cmd_delete_word_after(cmd_context_t *ctx);
int cmd_delete_word_before(cmd_context_t *ctx);
int cmd_drop_cursor_column(cmd_context_t *ctx);
int cmd_drop_lettered_mark(cmd_context_t *ctx);
int cmd_drop_sleeping_cursor(cmd_context_t *ctx);
int cmd_find_word(cmd_context_t *ctx);
int cmd_fsearch(cmd_context_t *ctx);
int cmd_fsearch_fzy(cmd_context_t *ctx);
int cmd_goto(cmd_context_t *ctx);
int cmd_goto_lettered_mark(cmd_context_t *ctx);
int cmd_grep(cmd_context_t *ctx);
int cmd_indent(cmd_context_t *ctx);
int cmd_insert_data(cmd_context_t *ctx);
int cmd_insert_newline_above(cmd_context_t *ctx);
int cmd_insert_newline_below(cmd_context_t *ctx);
int cmd_insert_newline(cmd_context_t *ctx);
int cmd_insert_tab(cmd_context_t *ctx);
int cmd_isearch(cmd_context_t *ctx);
int cmd_jump(cmd_context_t *ctx);
int cmd_last(cmd_context_t *ctx);
int cmd_less(cmd_context_t *ctx);
int cmd_move_beginning(cmd_context_t *ctx);
int cmd_move_bol(cmd_context_t *ctx);
int cmd_move_bracket_back(cmd_context_t *ctx);
int cmd_move_bracket_forward(cmd_context_t *ctx);
int cmd_move_bracket_toggle(cmd_context_t *ctx);
int cmd_move_down(cmd_context_t *ctx);
int cmd_move_end(cmd_context_t *ctx);
int cmd_move_eol(cmd_context_t *ctx);
int cmd_move_left(cmd_context_t *ctx);
int cmd_move_page_down(cmd_context_t *ctx);
int cmd_move_page_up(cmd_context_t *ctx);
int cmd_move_relative(cmd_context_t *ctx);
int cmd_move_right(cmd_context_t *ctx);
int cmd_move_temp_anchor(cmd_context_t *ctx);
int cmd_move_to_line(cmd_context_t *ctx);
int cmd_move_to_offset(cmd_context_t *ctx);
int cmd_move_until_back(cmd_context_t *ctx);
int cmd_move_until_forward(cmd_context_t *ctx);
int cmd_move_up(cmd_context_t *ctx);
int cmd_move_word_back(cmd_context_t *ctx);
int cmd_move_word_forward(cmd_context_t *ctx);
int cmd_next(cmd_context_t *ctx);
int cmd_noop(cmd_context_t *ctx);
int cmd_open_file(cmd_context_t *ctx);
int cmd_open_new(cmd_context_t *ctx);
int cmd_open_replace_file(cmd_context_t *ctx);
int cmd_open_replace_new(cmd_context_t *ctx);
int cmd_outdent(cmd_context_t *ctx);
int cmd_perl(cmd_context_t *ctx);
int cmd_pop_kmap(cmd_context_t *ctx);
int cmd_prev(cmd_context_t *ctx);
int cmd_push_kmap(cmd_context_t *ctx);
int cmd_quit(cmd_context_t *ctx);
int cmd_quit_without_saving(cmd_context_t *ctx);
int cmd_redo(cmd_context_t *ctx);
int cmd_redraw(cmd_context_t *ctx);
int cmd_remove_extra_cursors(cmd_context_t *ctx);
int cmd_repeat(cmd_context_t *ctx);
int cmd_replace(cmd_context_t *ctx);
int cmd_rfind_word(cmd_context_t *ctx);
int cmd_rsearch(cmd_context_t *ctx);
int cmd_save_as(cmd_context_t *ctx);
int cmd_save(cmd_context_t *ctx);
int cmd_search(cmd_context_t *ctx);
int cmd_search_next(cmd_context_t *ctx);
int cmd_search_prev(cmd_context_t *ctx);
int cmd_set_opt(cmd_context_t *ctx);
int cmd_shell(cmd_context_t *ctx);
int cmd_show_help(cmd_context_t *ctx);
int cmd_split_horizontal(cmd_context_t *ctx);
int cmd_split_vertical(cmd_context_t *ctx);
int cmd_suspend(cmd_context_t *ctx);
int cmd_swap_anchor(cmd_context_t *ctx);
int cmd_toggle_anchor(cmd_context_t *ctx);
int cmd_toggle_block(cmd_context_t *ctx);
int cmd_uncut(cmd_context_t *ctx);
int cmd_uncut_last(cmd_context_t *ctx);
int cmd_undo(cmd_context_t *ctx);
int cmd_viewport_bot(cmd_context_t *ctx);
int cmd_viewport_mid(cmd_context_t *ctx);
int cmd_viewport_toggle(cmd_context_t *ctx);
int cmd_viewport_top(cmd_context_t *ctx);
int cmd_wake_sleeping_cursors(cmd_context_t *ctx);

// async functions
aproc_t *aproc_new(editor_t *editor, void *owner, aproc_t **owner_aproc, char *shell_cmd, int rw, aproc_cb_t fn_callback);
int aproc_set_owner(aproc_t *aproc, void *owner, aproc_t **owner_aproc);
int aproc_destroy(aproc_t *aproc, int preempt);
int aproc_drain_all(aproc_t *aprocs, int *ttyfd);

// uscript functions
uscript_t *uscript_run(editor_t *editor, char *path);
int uscript_destroy(uscript_t *uscript);

// util functions
int util_shell_exec(editor_t *editor, char *cmd, long timeout_s, char *input, size_t input_len, int setsid, char *opt_shell, char **optret_output, size_t *optret_output_len, int *optret_exit_code);
int util_popen2(char *cmd, int setsid, char *opt_shell, int *optret_fdread, int *optret_fdwrite, pid_t *optret_pid);
int util_get_bracket_pair(uint32_t ch, int *optret_is_closing);
int util_is_file(char *path, char *opt_mode, FILE **optret_file);
int util_is_dir(char *path);
int util_pcre_match(char *re, char *subject, int subject_len, char **optret_capture, int *optret_capture_len);
int util_pcre_replace(char *re, char *subj, char *repl, char **ret_result, int *ret_result_len);
int util_timeval_is_gt(struct timeval *a, struct timeval *b);
char *util_escape_shell_arg(char *str, int l);
void util_expand_tilde(char *path, int path_len, char **ret_path, int *ret_path_len);
int tb_printf_rect(bview_rect_t rect, int x, int y, uint16_t fg, uint16_t bg, const char *fmt, ...);
int tb_printf_attr(bview_rect_t rect, int x, int y, const char *fmt, ...);

// Globals
extern editor_t _editor;

// Macros
#define MLE_VERSION "1.7.0-dev"

#define MLE_OK 0
#define MLE_ERR 1

#define MLE_PROMPT_YES "yes"
#define MLE_PROMPT_NO "no"
#define MLE_PROMPT_ALL "all"

#define MLE_DEFAULT_TAB_WIDTH 4
#define MLE_DEFAULT_TAB_TO_SPACE 1
#define MLE_DEFAULT_TRIM_PASTE 1
#define MLE_DEFAULT_AUTO_INDENT 0
#define MLE_DEFAULT_MACRO_TOGGLE_KEY "M-r"
#define MLE_DEFAULT_HILI_BRACKET_PAIRS 1
#define MLE_DEFAULT_READ_RC_FILE 1
#define MLE_DEFAULT_SOFT_WRAP 0
#define MLE_DEFAULT_COARSE_UNDO 0
#define MLE_DEFAULT_MOUSE_SUPPORT 0

#define MLE_LOG_ERR(fmt, ...) do { \
    fprintf(stderr, (fmt), __VA_ARGS__); \
} while (0)

#define MLE_SET_ERR(editor, fmt, ...) do { \
    snprintf((editor)->errstr, MLE_ERRSTR_SIZE, (fmt), __VA_ARGS__); \
} while (0)

#define MLE_SET_INFO(editor, fmt, ...) do { \
    snprintf((editor)->infostr, MLE_ERRSTR_SIZE, (fmt), __VA_ARGS__); \
} while (0)

#define MLE_RETURN_ERR(editor, fmt, ...) do { \
    MLE_SET_ERR((editor), (fmt), __VA_ARGS__); \
    return MLE_ERR; \
} while (0)

#define MLE_MIN(a,b) (((a)<(b)) ? (a) : (b))
#define MLE_MAX(a,b) (((a)>(b)) ? (a) : (b))

#define MLE_BVIEW_IS_EDIT(bview) ((bview)->type == MLE_BVIEW_TYPE_EDIT)
#define MLE_BVIEW_IS_MENU(bview) ((bview)->is_menu && MLE_BVIEW_IS_EDIT(bview))
#define MLE_BVIEW_IS_POPUP(bview) ((bview)->type == MLE_BVIEW_TYPE_POPUP)
#define MLE_BVIEW_IS_STATUS(bview) ((bview)->type == MLE_BVIEW_TYPE_STATUS)
#define MLE_BVIEW_IS_PROMPT(bview) ((bview)->type == MLE_BVIEW_TYPE_PROMPT)

#define MLE_MARK_COL_TO_VCOL(pmark) ( \
    (pmark)->col >= (pmark)->bline->char_count \
    ? (pmark)->bline->char_vwidth \
    : ( (pmark)->col <= 0 ? 0 : (pmark)->bline->chars[(pmark)->col].vcol ) \
)

#define MLE_COL_TO_VCOL(pline, pcol, pmax) ( \
    (pcol) >= (pline)->char_count \
    ? (pmax) \
    : ( (pcol) <= 0 ? 0 : (pline)->chars[(pcol)].vcol ) \
)

// Setter macros for kinput structs
#define MLE_KINPUT_SET_EX(pi, pfill, pmod, pch, pkey) do { \
    memset(&(pi), (pfill), sizeof(pi)); \
    (pi).mod = (pmod); \
    (pi).ch = (pch); \
    (pi).key = (pkey); \
} while(0)
#define MLE_KINPUT_SET(pi, pmod, pch, pkey) MLE_KINPUT_SET_EX(pi, 0x00, pmod, pch, pkey)
#define MLE_KINPUT_SET_SPECIAL(pi, pmod)    MLE_KINPUT_SET_EX(pi, 0xff, pmod, 0xffffffff, 0xffff)
#define MLE_KINPUT_SET_NUMERIC(pi)          MLE_KINPUT_SET_SPECIAL(pi, 0x40)
#define MLE_KINPUT_SET_WILDCARD(pi)         MLE_KINPUT_SET_SPECIAL(pi, 0x80)
#define MLE_KINPUT_COPY(pi, pj)             memcpy(&(pi), &(pj), sizeof(pi))

#define MLE_LINENUM_TYPE_ABS 0
#define MLE_LINENUM_TYPE_REL 1
#define MLE_LINENUM_TYPE_BOTH 2

#define MLE_PARAM_WILDCARD(pctx, pn) ( \
    (pn) < (pctx)->wildcard_params_len \
    ? (pctx)->wildcard_params[(pn)] \
    : 0 \
)

#define MLE_BRACKET_PAIR_MAX_SEARCH 10000

#define MLE_RE_WORD_FORWARD "((?<=\\w)\\W|$)"
#define MLE_RE_WORD_BACK "((?<=\\W)\\w|^)"

// TODO rename these inline
#define tb_change_cell          tb_set_cell
#define tb_put_cell(x, y, c)    tb_set_cell((x), (y), (c)
#define tb_set_clear_attributes tb_set_clear_attrs
#define tb_select_input_mode    tb_set_input_mode
#define tb_select_output_mode   tb_set_output_mode

/*
TODO major changes
[ ] delete lua api
[ ] rewrite kmap (complex/unreadable; ** and ## sucks; kinput as hash key sucks; consolidate input_trail + pastebuf)
[ ] rewrite/generalize aproc+menu (too tightly coupled; a better solution possibly supersedes dte's errorfmt/compile)
[ ] rewrite buffer_set_mmapped to avoid huge mallocs
[ ] use wcwidth9 autc instead of relying on locale and wcwidth (tests/unit/test_bline_insert.c)
TODO review
[ ] review error checking, esp catch ENOSPC, malloc fail
[ ] review compiler warnings
TODO new features/improvements
[ ] add cmd_tabulate
[ ] add ## param to page_up/down (by half/third etc)
[ ] replace mark_set_pcre_capture with mark local
[ ] use editor prompt history when bview prompt history is empty
[ ] improve isearch kmap (next/prev history)
[ ] add mark stack (push, move around, pop to go back)
[ ] add last cmd status indicator
[ ] check if buffer exists by inode instead of path
[ ] move single-use macros out of mle.h
[ ] review use of multi_cursor_code
[ ] review use of MLE_RETURN_ERR
[ ] try to get rid of use_srules
TODO maybe
[ ] ?allow uscripts to preempt control, use shared uscriptfd
[ ] ?add vim emulation mode
[ ] ?make colors, status line, layout configurable
[ ] ?add multi-level undo/redo
*/

#endif
