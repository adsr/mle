#ifndef __MLBUF_H
#define __MLBUF_H

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pcre.h>
#include <utlist.h>

// Typedefs
typedef struct buffer_s buffer_t; // A buffer of text (stored as a linked list of blines)
typedef struct bline_s bline_t; // A line in a buffer
typedef struct bline_char_s bline_char_t; // Metadata about a character in a bline
typedef struct baction_s baction_t; // An insert or delete action (used for undo)
typedef struct mark_s mark_t; // A mark in a buffer
typedef struct srule_s srule_t; // A style rule
typedef struct srule_node_s srule_node_t; // A node in a list of style rules
typedef struct sblock_s sblock_t; // A style of a particular character
typedef struct str_s str_t; // A dynamically resizeable string
typedef void (*buffer_callback_t)(buffer_t *buffer, baction_t *action, void *udata);
typedef intmax_t bint_t;

// str_t
struct str_s {
    char *data;
    size_t len;
    size_t cap;
    ssize_t inc;
};

// buffer_t
struct buffer_s {
    bline_t *first_line;
    bline_t *last_line;
    bint_t byte_count;
    bint_t line_count;
    srule_node_t *single_srules;
    srule_node_t *multi_srules;
    srule_node_t *range_srules;
    baction_t *actions;
    baction_t *action_tail;
    baction_t *action_undone;
    str_t registers[26];
    mark_t *lettered_marks[26];
    char *path;
    struct stat st;
    int is_unsaved;
    char *data;
    bint_t data_len;
    int is_data_dirty;
    int ref_count;
    int tab_width;
    buffer_callback_t callback;
    void *callback_udata;
    int mmap_fd;
    char *mmap;
    size_t mmap_len;
    bline_char_t *slabbed_chars;
    bline_t *slabbed_blines;
    int num_applied_srules;
    int is_in_open;
    int is_in_callback;
    int is_style_disabled;
    int _is_in_undo;
};

// bline_t
struct bline_s {
    buffer_t *buffer;
    char *data;
    bint_t data_len;
    bint_t data_cap;
    bint_t line_index;
    bint_t char_count;
    bint_t char_vwidth;
    bline_char_t *chars;
    bint_t chars_cap;
    mark_t *marks;
    srule_t *bol_rule;
    srule_t *eol_rule;
    int is_chars_dirty;
    int is_slabbed;
    int is_data_slabbed;
    bline_t *next;
    bline_t *prev;
};

// sblock_t
struct sblock_s {
    uint16_t fg;
    uint16_t bg;
};

// bline_char_t
struct bline_char_s {
    uint32_t ch;
    int len;
    bint_t index;
    bint_t vcol;
    bint_t index_to_vcol; // accessed via >chars[index], not >chars[char]
    sblock_t style;
};

// baction_t
struct baction_s {
    int type; // MLBUF_BACTION_TYPE_*
    buffer_t *buffer;
    bline_t *start_line;
    bint_t start_line_index;
    bint_t start_col;
    bline_t *maybe_end_line;
    bint_t maybe_end_line_index;
    bint_t maybe_end_col;
    bint_t byte_delta;
    bint_t char_delta;
    bint_t line_delta;
    char *data;
    bint_t data_len;
    baction_t *next;
    baction_t *prev;
};

// mark_t
struct mark_s {
    bline_t *bline;
    bint_t col;
    bint_t target_col;
    srule_t *range_srule;
    char letter;
    mark_t *next;
    mark_t *prev;
    int lefty;
};

// srule_t
struct srule_s {
    int type; // MLBUF_SRULE_TYPE_*
    char *re;
    char *re_end;
    pcre *cre;
    pcre *cre_end;
    pcre_extra *crex;
    pcre_extra *crex_end;
    mark_t *range_a;
    mark_t *range_b;
    sblock_t style;
};

// srule_node_t
struct srule_node_s {
    srule_t *srule;
    srule_node_t *next;
    srule_node_t *prev;
};

// buffer functions
buffer_t *buffer_new();
buffer_t *buffer_new_open(char *path);
mark_t *buffer_add_mark(buffer_t *self, bline_t *maybe_line, bint_t maybe_col);
mark_t *buffer_add_mark_ex(buffer_t *self, char letter, bline_t *maybe_line, bint_t maybe_col);
int buffer_get_lettered_mark(buffer_t *self, char letter, mark_t **ret_mark);
int buffer_destroy_mark(buffer_t *self, mark_t *mark);
int buffer_open(buffer_t *self, char *path);
int buffer_save(buffer_t *self);
int buffer_save_as(buffer_t *self, char *path, bint_t *optret_nbytes);
int buffer_write_to_file(buffer_t *self, FILE *fp, size_t *optret_nbytes);
int buffer_write_to_fd(buffer_t *self, int fd, size_t *optret_nbytes);
int buffer_get(buffer_t *self, char **ret_data, bint_t *ret_data_len);
int buffer_clear(buffer_t *self);
int buffer_set(buffer_t *self, char *data, bint_t data_len);
int buffer_set_mmapped(buffer_t *self, char *data, bint_t data_len);
int buffer_substr(buffer_t *self, bline_t *start_line, bint_t start_col, bline_t *end_line, bint_t end_col, char **ret_data, bint_t *ret_data_len, bint_t *ret_nchars);
int buffer_insert(buffer_t *self, bint_t offset, char *data, bint_t data_len, bint_t *optret_num_chars);
int buffer_delete(buffer_t *self, bint_t offset, bint_t num_chars);
int buffer_replace(buffer_t *self, bint_t offset, bint_t num_chars, char *data, bint_t data_len);
int buffer_insert_w_bline(buffer_t *self, bline_t *start_line, bint_t start_col, char *data, bint_t data_len, bint_t *optret_num_chars);
int buffer_delete_w_bline(buffer_t *self, bline_t *start_line, bint_t start_col, bint_t num_chars);
int buffer_replace_w_bline(buffer_t *self, bline_t *start_line, bint_t start_col, bint_t num_chars, char *data, bint_t data_len);
int buffer_get_bline(buffer_t *self, bint_t line_index, bline_t **ret_bline);
int buffer_get_bline_w_hint(buffer_t *self, bint_t line_index, bline_t *opt_hint, bline_t **ret_bline);
int buffer_get_bline_col(buffer_t *self, bint_t offset, bline_t **ret_bline, bint_t *ret_col);
int buffer_get_offset(buffer_t *self, bline_t *bline, bint_t col, bint_t *ret_offset);
int buffer_undo(buffer_t *self);
int buffer_redo(buffer_t *self);
int buffer_add_srule(buffer_t *self, srule_t *srule);
int buffer_remove_srule(buffer_t *self, srule_t *srule);
int buffer_set_callback(buffer_t *self, buffer_callback_t fn_cb, void *udata);
int buffer_set_tab_width(buffer_t *self, int tab_width);
int buffer_set_styles_enabled(buffer_t *self, int is_enabled);
int buffer_apply_styles(buffer_t *self, bline_t *start_line, bint_t line_delta);
int buffer_register_set(buffer_t *self, char reg, char *data, size_t data_len);
int buffer_register_append(buffer_t *self, char reg, char *data, size_t data_len);
int buffer_register_prepend(buffer_t *self, char reg, char *data, size_t data_len);
int buffer_register_clear(buffer_t *self, char reg);
int buffer_register_get(buffer_t *self, char reg, int dup, char **ret_data, size_t *ret_data_len);
int buffer_destroy(buffer_t *self);

// bline functions
int bline_insert(bline_t *self, bint_t col, char *data, bint_t data_len, bint_t *ret_num_chars);
int bline_delete(bline_t *self, bint_t col, bint_t num_chars);
int bline_replace(bline_t *self, bint_t col, bint_t num_chars, char *data, bint_t data_len);
int bline_get_col(bline_t *self, bint_t index, bint_t *ret_col);
int bline_get_col_from_vcol(bline_t *self, bint_t vcol, bint_t *ret_col);
int bline_count_chars(bline_t *bline);

// mark functions
int mark_clone(mark_t *self, mark_t **ret_mark);
int mark_clone_w_letter(mark_t *self, char letter, mark_t **ret_mark);
int mark_delete_after(mark_t *self, bint_t num_chars);
int mark_delete_before(mark_t *self, bint_t num_chars);
int mark_delete_between_mark(mark_t *self, mark_t *other);
int mark_destroy(mark_t *self);
int mark_find_bracket_pair(mark_t *self, bint_t max_chars, bline_t **ret_line, bint_t *ret_col, bint_t *ret_brkt);
int mark_find_bracket_top(mark_t *self, bint_t max_chars, bline_t **ret_line, bint_t *ret_col, bint_t *ret_brkt);
int mark_find_next_cre(mark_t *self, pcre *cre, bline_t **ret_line, bint_t *ret_col, bint_t *ret_num_chars);
int mark_find_next_re(mark_t *self, char *re, bint_t re_len, bline_t **ret_line, bint_t *ret_col, bint_t *ret_num_chars);
int mark_find_next_str(mark_t *self, char *str, bint_t str_len, bline_t **ret_line, bint_t *ret_col, bint_t *ret_num_chars);
int mark_find_prev_cre(mark_t *self, pcre *cre, bline_t **ret_line, bint_t *ret_col, bint_t *ret_num_chars);
int mark_find_prev_re(mark_t *self, char *re, bint_t re_len, bline_t **ret_line, bint_t *ret_col, bint_t *ret_num_chars);
int mark_find_prev_str(mark_t *self, char *str, bint_t str_len, bline_t **ret_line, bint_t *ret_col, bint_t *ret_num_chars);
int mark_get_between_mark(mark_t *self, mark_t *other, char **ret_str, bint_t *ret_str_len);
int mark_get_char_after(mark_t *self, uint32_t *ret_char);
int mark_get_char_before(mark_t *self, uint32_t *ret_char);
int mark_get_nchars_between(mark_t *self, mark_t *other, bint_t *ret_nchars);
int mark_get_offset(mark_t *self, bint_t *ret_offset);
int mark_insert_after(mark_t *self, char *data, bint_t data_len);
int mark_insert_before(mark_t *self, char *data, bint_t data_len);
int mark_is_after_col_minus_lefties(mark_t *self, bint_t col);
int mark_is_at_bol(mark_t *self);
int mark_is_at_eol(mark_t *self);
int mark_is_at_word_bound(mark_t *self, int side);
int mark_is_eq(mark_t *self, mark_t *other);
int mark_is_gte(mark_t *self, mark_t *other);
int mark_is_gt(mark_t *self, mark_t *other);
int mark_is_lte(mark_t *self, mark_t *other);
int mark_is_lt(mark_t *self, mark_t *other);
int mark_is_between(mark_t *self, mark_t *ma, mark_t *mb);
int mark_join(mark_t *self, mark_t *other);
int mark_move_beginning(mark_t *self);
int mark_move_bol(mark_t *self);
int mark_move_bracket_pair_ex(mark_t *self, bint_t max_chars, bline_t **optret_line, bint_t *optret_col, bint_t *optret_num_chars);
int mark_move_bracket_pair(mark_t *self, bint_t max_chars);
int mark_move_bracket_top_ex(mark_t *self, bint_t max_chars, bline_t **optret_line, bint_t *optret_col, bint_t *optret_num_chars);
int mark_move_bracket_top(mark_t *self, bint_t max_chars);
int mark_move_by(mark_t *self, bint_t char_delta);
int mark_move_col(mark_t *self, bint_t col);
int mark_move_end(mark_t *self);
int mark_move_eol(mark_t *self);
int mark_move_next_cre_ex(mark_t *self, pcre *cre, bline_t **optret_line, bint_t *optret_col, bint_t *optret_num_chars);
int mark_move_next_cre(mark_t *self, pcre *cre);
int mark_move_next_cre_nudge(mark_t *self, pcre *cre);
int mark_move_next_re_ex(mark_t *self, char *re, bint_t re_len, bline_t **optret_line, bint_t *optret_col, bint_t *optret_num_chars);
int mark_move_next_re(mark_t *self, char *re, bint_t re_len);
int mark_move_next_re_nudge(mark_t *self, char *re, bint_t re_len);
int mark_move_next_str_ex(mark_t *self, char *str, bint_t str_len, bline_t **optret_line, bint_t *optret_col, bint_t *optret_num_chars);
int mark_move_next_str(mark_t *self, char *str, bint_t str_len);
int mark_move_next_str_nudge(mark_t *self, char *str, bint_t str_len);
int mark_move_offset(mark_t *self, bint_t offset);
int mark_move_prev_cre_ex(mark_t *self, pcre *cre, bline_t **optret_line, bint_t *optret_col, bint_t *optret_num_chars);
int mark_move_prev_cre(mark_t *self, pcre *cre);
int mark_move_prev_re_ex(mark_t *self, char *re, bint_t re_len, bline_t **optret_line, bint_t *optret_col, bint_t *optret_num_chars);
int mark_move_prev_re(mark_t *self, char *re, bint_t re_len);
int mark_move_prev_str_ex(mark_t *self, char *str, bint_t str_len, bline_t **optret_line, bint_t *optret_col, bint_t *optret_num_chars);
int mark_move_prev_str(mark_t *self, char *str, bint_t str_len);
int mark_move_to(mark_t *self, bint_t line_index, bint_t col);
int mark_move_to_w_bline(mark_t *self, bline_t *bline, bint_t col);
int mark_move_vert(mark_t *self, bint_t line_delta);
int mark_replace_between_mark(mark_t *self, mark_t *other, char *data, bint_t data_len);
int mark_replace(mark_t *self, bint_t num_chars, char *data, bint_t data_len);
int mark_set_pcre_capture(int *rc, int *ovector, int ovector_size);
int mark_swap_with_mark(mark_t *self, mark_t *other);

// srule functions
srule_t *srule_new_single(char *re, bint_t re_len, int caseless, uint16_t fg, uint16_t bg);
srule_t *srule_new_multi(char *re, bint_t re_len, char *re_end, bint_t re_end_len, uint16_t fg, uint16_t bg);
srule_t *srule_new_range(mark_t *range_a, mark_t *range_b, uint16_t fg, uint16_t bg);
int srule_destroy(srule_t *srule);

// utf8 functions
int utf8_char_length(char c);
int utf8_char_to_unicode(uint32_t *out, const char *c, const char *stop);
int utf8_unicode_to_char(char *out, uint32_t c);

// util functions
void *recalloc(void *ptr, size_t orig_num, size_t new_num, size_t el_size);
void _mark_mark_move_inner(mark_t *mark, bline_t *bline_target, bint_t col, int do_set_target);
void str_append_stop(str_t *str, char *data, char *data_stop);
void str_append(str_t *str, char *data);
void str_append_char(str_t *str, char c);
void str_append_len(str_t *str, char *data, size_t data_len);
void str_prepend_stop(str_t *str, char *data, char *data_stop);
void str_prepend(str_t *str, char *data);
void str_prepend_len(str_t *str, char *data, size_t data_len);
void str_set(str_t *str, char *data);
void str_set_len(str_t *str, char *data, size_t data_len);
void str_put_len(str_t *str, char *data, size_t data_len, int is_prepend);
void str_ensure_cap(str_t *str, size_t cap);
void str_clear(str_t *str);
void str_free(str_t *str);
void str_sprintf(str_t *str, const char *fmt, ...);
void str_append_replace_with_backrefs(str_t *str, char *subj, char *repl, int pcre_rc, int *pcre_ovector, int pcre_ovecsize);

// Macros
#define MLBUF_DEBUG 1

// #define MLBUF_LARGE_FILE_SIZE 10485760
#define MLBUF_LARGE_FILE_SIZE 0

#define MLBUF_OK 0
#define MLBUF_ERR 1

#define MLBUF_BACTION_TYPE_INSERT 0
#define MLBUF_BACTION_TYPE_DELETE 1

#define MLBUF_SRULE_TYPE_SINGLE 0
#define MLBUF_SRULE_TYPE_MULTI 1
#define MLBUF_SRULE_TYPE_RANGE 2

#define MLBUF_MIN(a,b) (((a)<(b)) ? (a) : (b))
#define MLBUF_MAX(a,b) (((a)>(b)) ? (a) : (b))

#define MLBUF_BLINE_DATA_STOP(bline) ((bline)->data + ((bline)->data_len))

#define MLBUF_DEBUG_PRINTF(fmt, ...) do { \
    if (MLBUF_DEBUG) { \
        fprintf(stderr, "%lu ", time(0)); \
        fprintf(stderr, (fmt), __VA_ARGS__); \
        fflush(stderr); \
    } \
} while (0)

#define MLBUF_BLINE_ENSURE_CHARS(b) do { \
    if ((b)->is_chars_dirty) { \
        bline_count_chars(b); \
    } \
} while (0)

#define MLBUF_MAKE_GT_EQ0(v) if ((v) < 0) v = 0

#define MLBUF_INIT_PCRE_EXTRA(n) \
    pcre_extra n = { .flags = PCRE_EXTRA_MATCH_LIMIT_RECURSION, .match_limit_recursion = 256 }

#define MLBUF_ENSURE_AZ(c) \
    if ((c) < 'a' || (c) > 'z') return MLBUF_ERR

#define MLBUF_REG_PTR(buf, lett) \
    &((buf)->registers[(lett) - 'a'])

#define MLBUF_LETT_MARK(buf, lett) \
    (buf)->lettered_marks[(lett) - 'a']

#ifndef PCRE_STUDY_JIT_COMPILE
#define PCRE_STUDY_JIT_COMPILE 0
#endif

#ifdef PCRE_CONFIG_JIT
#define pcre_free_study_ex pcre_free_study
#else
#define pcre_free_study_ex pcre_free
#endif

#endif
