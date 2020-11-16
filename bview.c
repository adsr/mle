#include <math.h>
#include <unistd.h>
#include <wctype.h>
#include "mle.h"

static int _bview_rectify_viewport_dim(bview_t *self, bline_t *bline, bint_t vpos, int dim_scope, int dim_size, bint_t *view_vpos);
static void _bview_init(bview_t *self, buffer_t *buffer);
static void _bview_init_resized(bview_t *self);
static kmap_t *_bview_get_init_kmap(editor_t *editor);
static void _bview_buffer_callback(buffer_t *buffer, baction_t *action, void *udata);
static int _bview_set_linenum_width(bview_t *self);
static void _bview_deinit(bview_t *self);
static void _bview_set_tab_width(bview_t *self, int tab_width);
static void _bview_fix_path(bview_t *self, char *path, int path_len, char **ret_path, int *ret_path_len, bint_t *ret_line_num);
static void _bview_expand_tilde(bview_t *self, char *path, int path_len, char **ret_path, int *ret_path_len);
static buffer_t *_bview_open_buffer(bview_t *self, char *opt_path, int opt_path_len);
static void _bview_draw_prompt(bview_t *self);
static void _bview_draw_status(bview_t *self);
static void _bview_draw_edit(bview_t *self, int x, int y, int w, int h);
static void _bview_draw_bline(bview_t *self, bline_t *bline, int rect_y, bline_t **optret_bline, int *optret_rect_y);
static void _bview_highlight_bracket_pair(bview_t *self, mark_t *mark);
static int _bview_is_in_range(bline_t *bline, bint_t col, srule_t **ret_srule);

// Create a new bview
bview_t *bview_new(editor_t *editor, int type, char *opt_path, int opt_path_len, buffer_t *opt_buffer) {
    static int id = 0;
    bview_t *self;
    buffer_t *buffer;

    // Allocate and init bview
    self = calloc(1, sizeof(bview_t));
    self->editor = editor;
    self->type = type;
    self->path = strndup(opt_path, opt_path_len);
    self->rect_caption.fg = TB_WHITE;
    self->rect_caption.bg = TB_BLACK;
    self->rect_lines.fg = TB_YELLOW;
    self->rect_lines.bg = TB_BLACK;
    self->rect_margin_left.fg = TB_RED;
    self->rect_margin_right.fg = TB_RED;
    self->rect_buffer.h = 10; // TODO hack to fix _bview_set_linenum_width before bview_resize
    self->tab_width = editor->tab_width;
    self->tab_to_space = editor->tab_to_space;
    self->soft_wrap = editor->soft_wrap;
    self->viewport_scope_x = editor->viewport_scope_x;
    self->viewport_scope_y = editor->viewport_scope_y;
    self->id = (id++);
    getcwd(self->init_cwd, PATH_MAX + 1);

    // Open buffer
    if (opt_buffer) {
        buffer = opt_buffer;
    } else {
        buffer = _bview_open_buffer(self, opt_path, opt_path_len);
    }
    _bview_init(self, buffer);

    return self;
}

// Open a buffer in an existing bview
int bview_open(bview_t *self, char *path, int path_len) {
    buffer_t *buffer;
    buffer = _bview_open_buffer(self, path, path_len);
    if (self->path) free(self->path);
    self->path = strndup(path, path_len);
    _bview_init(self, buffer);
    bview_resize(self, self->x, self->y, self->w, self->h);
    return MLE_OK;
}

// Free a bview
int bview_destroy(bview_t *self) {
    _bview_deinit(self);
    if (self->path) free(self->path);
    // TODO ensure everything freed
    free(self);
    return MLE_OK;
}

// Move and resize a bview to the given position and dimensions
int bview_resize(bview_t *self, int x, int y, int w, int h) {
    int aw, ah;

    self->x = x;
    self->y = y;
    self->w = w;
    self->h = h;

    aw = w;
    ah = h;

    if (self->split_child) {
        if (self->split_is_vertical) {
            aw = MLE_MAX(1, (int)((float)aw * self->split_factor));
        } else {
            ah = MLE_MAX(1, (int)((float)ah * self->split_factor));
        }
    }

    if (MLE_BVIEW_IS_EDIT(self)) {
        self->rect_caption.x = x;
        self->rect_caption.y = y;
        self->rect_caption.w = aw;
        self->rect_caption.h = 1;

        self->rect_lines.x = x;
        self->rect_lines.y = y + 1;
        self->rect_lines.w = self->linenum_width;
        self->rect_lines.h = ah - 1;

        self->rect_margin_left.x = x + self->linenum_width;
        self->rect_margin_left.y = y + 1;
        self->rect_margin_left.w = 1;
        self->rect_margin_left.h = ah - 1;

        self->rect_buffer.x = x + self->linenum_width + 1;
        self->rect_buffer.y = y + 1;
        self->rect_buffer.w = MLE_MAX(1, aw - (self->linenum_width + 1 + 1));
        self->rect_buffer.h = ah - 1;

        self->rect_margin_right.x = x + (aw - 1);
        self->rect_margin_right.y = y + 1;
        self->rect_margin_right.w = 1;
        self->rect_margin_right.h = ah - 1;
    } else {
        self->rect_buffer.x = x;
        self->rect_buffer.y = y;
        self->rect_buffer.w = aw;
        self->rect_buffer.h = ah;
    }

    if (self->split_child) {
        bview_resize(
            self->split_child,
            x + (self->split_is_vertical ? aw : 0),
            y + (self->split_is_vertical ? 0 : ah),
            w - (self->split_is_vertical ? aw : 0),
            h - (self->split_is_vertical ? 0 : ah)
        );
    }

    if (!self->is_resized) {
        _bview_init_resized(self);
        self->is_resized = 1;
    }

    bview_rectify_viewport(self);

    return MLE_OK;
}

// Return top-most split_parent of a bview
bview_t *bview_get_split_root(bview_t *self) {
    bview_t *root;
    root = self;
    while (root->split_parent) {
        root = root->split_parent;
    }
    return root;
}

// Draw bview to screen
int bview_draw(bview_t *self) {
    if (MLE_BVIEW_IS_PROMPT(self)) {
        _bview_draw_prompt(self);
    } else if (MLE_BVIEW_IS_STATUS(self)) {
        _bview_draw_status(self);
    }
    _bview_draw_edit(self, self->x, self->y, self->w, self->h);
    return MLE_OK;
}

// Set cursor to screen
int bview_draw_cursor(bview_t *self, int set_real_cursor) {
    cursor_t *cursor;
    mark_t *mark;
    int screen_x;
    int screen_y;
    struct tb_cell *cell;
    DL_FOREACH(self->cursors, cursor) {
        mark = cursor->mark;
        if (bview_get_screen_coords(self, mark, &screen_x, &screen_y, &cell) != MLE_OK) {
            // Out of bounds
            continue;
        }
        if (set_real_cursor && cursor == self->active_cursor) {
            // Set terminal cursor
            tb_set_cursor(screen_x, screen_y);
        } else {
            // Set fake cursor
            tb_change_cell(screen_x, screen_y, cell->ch, cell->fg, cell->bg | (cursor->is_asleep ? TB_RED : TB_CYAN)); // TODO configurable
        }
        if (self->editor->highlight_bracket_pairs) {
            _bview_highlight_bracket_pair(self, mark);
        }
    }
    return MLE_OK;
}

// Push a kmap
int bview_push_kmap(bview_t *bview, kmap_t *kmap) {
    kmap_node_t *node;
    node = calloc(1, sizeof(kmap_node_t));
    node->kmap = kmap;
    node->bview = bview;
    DL_APPEND(bview->kmap_stack, node);
    bview->kmap_tail = node;
    return MLE_OK;
}

// Pop a kmap
int bview_pop_kmap(bview_t *bview, kmap_t **optret_kmap) {
    kmap_node_t *node_to_pop;
    node_to_pop = bview->kmap_tail;
    if (!node_to_pop) {
        return MLE_ERR;
    }
    if (optret_kmap) {
        *optret_kmap = node_to_pop->kmap;
    }
    bview->kmap_tail = node_to_pop->prev != node_to_pop ? node_to_pop->prev : NULL;
    DL_DELETE(bview->kmap_stack, node_to_pop);
    free(node_to_pop);
    return MLE_OK;
}

// Split a bview
int bview_split(bview_t *self, int is_vertical, float factor, bview_t **optret_bview) {
    bview_t *child;

    if (self->split_child) {
        MLE_RETURN_ERR(self->editor, "bview %p is already split", (void*)self);
    } else if (!MLE_BVIEW_IS_EDIT(self)) {
        MLE_RETURN_ERR(self->editor, "bview %p is not an edit bview", (void*)self);
    }

    // Make child
    editor_open_bview(self->editor, self, self->type, NULL, 0, 1, 0, 1, self->buffer, &child);
    child->split_parent = self;
    self->split_child = child;
    self->split_factor = factor;
    self->split_is_vertical = is_vertical;

    // Move cursor to same position
    mark_move_to(child->active_cursor->mark, self->active_cursor->mark->bline->line_index, self->active_cursor->mark->col);
    bview_center_viewport_y(child);

    // Resize self
    bview_resize(self, self->x, self->y, self->w, self->h);

    if (optret_bview) {
        *optret_bview = child;
    }
    return MLE_OK;
}

// Return number of active cursors
int bview_get_active_cursor_count(bview_t *self) {
    int count;
    cursor_t *cursor;
    count = 0;
    DL_FOREACH(self->cursors, cursor) {
        if (!cursor->is_asleep) {
            count += 1;
        }
    }
    return count;
}

// Add a cursor to a bview
int bview_add_cursor(bview_t *self, bline_t *opt_bline, bint_t opt_col, cursor_t **optret_cursor) {
    cursor_t *cursor;
    cursor = calloc(1, sizeof(cursor_t));
    cursor->bview = self;
    if (!opt_bline) opt_bline = self->buffer->first_line;
    if (opt_col < 0) opt_col = 0;
    cursor->mark = buffer_add_mark(self->buffer, opt_bline, opt_col);
    DL_APPEND(self->cursors, cursor);
    if (!self->active_cursor) {
        self->active_cursor = cursor;
    }
    if (optret_cursor) {
        *optret_cursor = cursor;
    }
    return MLE_OK;
}

// Add sleeping cursor
int bview_add_cursor_asleep(bview_t *self, bline_t *opt_bline, bint_t opt_col, cursor_t **optret_cursor) {
    cursor_t *cursor;
    bview_add_cursor(self, opt_bline, opt_col, &cursor);
    cursor->is_asleep = 1;
    if (optret_cursor) *optret_cursor = cursor;
    return MLE_OK;
}

// Wake all sleeping cursors
int bview_wake_sleeping_cursors(bview_t *self) {
    cursor_t *cursor;
    DL_FOREACH(self->cursors, cursor) {
        if (cursor->is_asleep) {
            cursor->is_asleep = 0;
        }
    }
    return MLE_OK;
}

// Remove all cursors except one
int bview_remove_cursors_except(bview_t *self, cursor_t *one) {
    cursor_t *cursor;
    cursor_t *cursor_tmp;
    DL_FOREACH_SAFE(self->cursors, cursor, cursor_tmp) {
        if (cursor != one) {
            bview_remove_cursor(self, cursor);
        }
    }
    return MLE_OK;
}

// Remove a cursor from a bview
int bview_remove_cursor(bview_t *self, cursor_t *cursor) {
    cursor_t *el;
    cursor_t *tmp;
    DL_FOREACH_SAFE(self->cursors, el, tmp) {
        if (el == cursor) {
            self->active_cursor = el->prev && el->prev != el ? el->prev : el->next;
            DL_DELETE(self->cursors, el);
            if (el->sel_rule) {
                buffer_remove_srule(el->bview->buffer, el->sel_rule);
                srule_destroy(el->sel_rule);
                el->sel_rule = NULL;
            }
            if (el->cut_buffer) free(el->cut_buffer);
            free(el);
            return MLE_OK;
        }
    }
    return MLE_ERR;
}

// Set viewport y safely
int bview_set_viewport_y(bview_t *self, bint_t y, int do_rectify) {
    if (y < 0) {
        y = 0;
    } else if (y >= self->buffer->line_count) {
        y = self->buffer->line_count - 1;
    }
    self->viewport_y = y;
    if (do_rectify) bview_rectify_viewport(self);
    buffer_get_bline_w_hint(self->buffer, self->viewport_y, self->viewport_bline, &self->viewport_bline);
    return MLE_OK;
}

// Center the viewport vertically
int bview_center_viewport_y(bview_t *self) {
    bint_t center;
    center = self->active_cursor->mark->bline->line_index - self->rect_buffer.h/2;
    if (center < 0) center = 0;
    return bview_set_viewport_y(self, center, 1);
}

// Zero the viewport vertically
int bview_zero_viewport_y(bview_t *self) {
    return bview_set_viewport_y(self, self->active_cursor->mark->bline->line_index, 1);
}

// Maximize the viewport vertically
int bview_max_viewport_y(bview_t *self) {
    bint_t max;
    max = self->active_cursor->mark->bline->line_index - self->rect_buffer.h;
    if (max < 0) max = 0;
    return bview_set_viewport_y(self, max, 1);
}

// Rectify the viewport
int bview_rectify_viewport(bview_t *self) {
    mark_t *mark;
    mark = self->active_cursor->mark;

    // Rectify each dimension of the viewport
    MLBUF_BLINE_ENSURE_CHARS(mark->bline);
    _bview_rectify_viewport_dim(self, mark->bline, MLE_MARK_COL_TO_VCOL(mark), self->viewport_scope_x, self->rect_buffer.w, &self->viewport_x_vcol);
    bline_get_col_from_vcol(mark->bline, self->viewport_x_vcol, &(self->viewport_x));

    if (_bview_rectify_viewport_dim(self, mark->bline, mark->bline->line_index, self->viewport_scope_y, self->rect_buffer.h, &self->viewport_y)) {
        // TODO viewport_y_vrow (soft-wrapped lines, code folding, etc)
        // Refresh viewport_bline
        buffer_get_bline_w_hint(self->buffer, self->viewport_y, self->viewport_bline, &self->viewport_bline);
    }

    return MLE_OK;
}

// Add a listener
int bview_add_listener(bview_t *self, bview_listener_cb_t callback, void *udata) {
    bview_listener_t *listener;
    listener = calloc(1, sizeof(bview_listener_t));
    listener->callback = callback;
    listener->udata = udata;
    DL_APPEND(self->listeners, listener);
    return MLE_OK;
}

// Remove and free a listener
int bview_destroy_listener(bview_t *self, bview_listener_t *listener) {
    DL_DELETE(self->listeners, listener);
    free(listener);
    return MLE_OK;
}

// Rectify a viewport dimension. Return 1 if changed, else 0.
static int _bview_rectify_viewport_dim(bview_t *self, bline_t *bline, bint_t vpos, int dim_scope, int dim_size, bint_t *view_vpos) {
    int rc;
    bint_t vpos_start;
    bint_t vpos_stop;
    (void)self;
    (void)bline;

    // Find bounds
    if (dim_scope < 0) {
        // Keep cursor at least `dim_scope` cells away from edge
        // Remember dim_scope is negative here
        dim_scope = MLE_MAX(dim_scope, ((dim_size / 2) * -1));
        vpos_start = *view_vpos - dim_scope; // N in from left edge
        vpos_stop = (*view_vpos + dim_size) + dim_scope; // N in from right edge
    } else {
        // Keep cursor within `dim_scope/2` cells of midpoint
        dim_scope = MLE_MIN(dim_scope, dim_size);
        vpos_start = (*view_vpos + (dim_size / 2)) - (int)floorf((float)dim_scope * 0.5); // -N/2 from midpoint
        vpos_stop = (*view_vpos + (dim_size / 2)) + (int)ceilf((float)dim_scope * 0.5); // +N/2 from midpoint
    }

    // Rectify
    rc = 1;
    if (vpos < vpos_start) {
        *view_vpos -= MLE_MIN(*view_vpos, vpos_start - vpos);
    } else if (vpos >= vpos_stop) {
        *view_vpos += ((vpos - vpos_stop) + 1);
    } else {
        rc = 0;
    }

    return rc;
}

// Init a bview with a buffer
static void _bview_init(bview_t *self, buffer_t *buffer) {
    cursor_t *cursor_tmp;
    kmap_t *kmap_init;

    _bview_deinit(self);

    // Reference buffer
    self->buffer = buffer;
    self->buffer->ref_count += 1;
    _bview_set_linenum_width(self);

    // Push normal mode
    kmap_init = _bview_get_init_kmap(self->editor);
    if (kmap_init != self->editor->kmap_normal) {
        // Make kmap_normal the bottom if init kmap isn't kmap_normal
        bview_push_kmap(self, self->editor->kmap_normal);
    }
    bview_push_kmap(self, kmap_init);

    // Set syntax
    bview_set_syntax(self, NULL);

    // Add a cursor
    bview_add_cursor(self, self->buffer->first_line, 0, &cursor_tmp);
}

// Invoked once after a bview has been resized for the first time
static void _bview_init_resized(bview_t *self) {
    // Move cursor to startup line if present
    if (self->startup_linenum > 0) {
        mark_move_to(self->active_cursor->mark, self->startup_linenum, 0);
        bview_center_viewport_y(self);
    }
}

// Return initial kmap to use
static kmap_t *_bview_get_init_kmap(editor_t *editor) {
    if (!editor->kmap_init) {
        if (editor->kmap_init_name) {
            HASH_FIND_STR(editor->kmap_map, editor->kmap_init_name, editor->kmap_init);
        }
        if (!editor->kmap_init) {
            editor->kmap_init = editor->kmap_normal;
        }
    }
    return editor->kmap_init;
}

// Called by mlbuf after edits
static void _bview_buffer_callback(buffer_t *buffer, baction_t *action, void *udata) {
    editor_t *editor;
    bview_t *self;
    bview_t *active;
    bview_listener_t *listener;

    self = (bview_t*)udata;
    editor = self->editor;
    active = editor->active;

    // Rectify viewport if edit was on active bview
    if (active->buffer == buffer) {
        bview_rectify_viewport(active);
    }

    if (action && action->line_delta != 0) {
        bview_t *bview;
        bview_t *tmp1;
        bview_t *tmp2;
        CDL_FOREACH_SAFE2(editor->all_bviews, bview, tmp1, tmp2, all_prev, all_next) {
            if (bview->buffer == buffer) {
                // Adjust linenum_width
                if (_bview_set_linenum_width(bview)) {
                    bview_resize(bview, bview->x, bview->y, bview->w, bview->h);
                }
                // Adjust viewport_bline
                buffer_get_bline_w_hint(bview->buffer, bview->viewport_y, bview->viewport_bline, &bview->viewport_bline);
            }
        }
    }

    // Call bview listeners
    DL_FOREACH(self->listeners, listener) {
        listener->callback(self, action, listener->udata);
    }

    // Notify event observers
    editor_notify_observers(editor, "buffer:baction", (void*)action);
}

// Set linenum_width and return 1 if changed
static int _bview_set_linenum_width(bview_t *self) {
    int orig;
    orig = self->linenum_width;
    self->abs_linenum_width = MLE_MAX(1, (int)(floor(log10((double)self->buffer->line_count))) + 1);
    if (self->editor->linenum_type != MLE_LINENUM_TYPE_ABS) {
        self->rel_linenum_width = MLE_MAX(
            self->editor->linenum_type == MLE_LINENUM_TYPE_BOTH ? 1 : self->abs_linenum_width,
            (int)(floor(log10((double)self->rect_buffer.h))) + 1
        );
    } else {
        self->rel_linenum_width = 0;
    }
    if (self->editor->linenum_type == MLE_LINENUM_TYPE_ABS) {
        self->linenum_width = self->abs_linenum_width;
    } else if (self->editor->linenum_type == MLE_LINENUM_TYPE_REL) {
        self->linenum_width = self->abs_linenum_width > self->rel_linenum_width ? self->abs_linenum_width : self->rel_linenum_width;
    } else if (self->editor->linenum_type == MLE_LINENUM_TYPE_BOTH) {
        self->linenum_width = self->abs_linenum_width + 1 + self->rel_linenum_width;
    }
    return orig == self->linenum_width ? 0 : 1;
}

// Deinit a bview
static void _bview_deinit(bview_t *self) {
    bview_listener_t *listener;
    bview_listener_t *listener_tmp;

    // Remove all kmaps
    while (self->kmap_tail) {
        bview_pop_kmap(self, NULL);
    }

    // Remove all syntax rules
    if (self->syntax) {
        srule_node_t *srule_node;
        buffer_set_styles_enabled(self->buffer, 0);
        DL_FOREACH(self->syntax->srules, srule_node) {
            buffer_remove_srule(self->buffer, srule_node->srule);
        }
        buffer_set_styles_enabled(self->buffer, 1);
    }

    // Remove all cursors
    while (self->active_cursor) {
        bview_remove_cursor(self, self->active_cursor);
    }

    // Destroy async proc
    if (self->aproc) {
        aproc_destroy(self->aproc, 1);
        self->aproc = NULL;
    }

    // Remove all listeners
    DL_FOREACH_SAFE(self->listeners, listener, listener_tmp) {
        bview_destroy_listener(self, listener);
    }

    // Dereference/free buffer
    if (self->buffer) {
        self->buffer->ref_count -= 1;
        if (self->buffer->ref_count < 1) {
            buffer_destroy(self->buffer);
        }
    }

    // Free last_search
    if (self->last_search) {
        free(self->last_search);
    }
}

// Set syntax on bview buffer
int bview_set_syntax(bview_t *self, char *opt_syntax) {
    syntax_t *syntax;
    syntax_t *use_syntax;
    srule_node_t *srule_node;

    // Only set syntax on edit bviews
    if (!MLE_BVIEW_IS_EDIT(self)) {
        return MLE_ERR;
    }

    use_syntax = NULL;
    if (opt_syntax) {
        // Set by opt_syntax
        HASH_FIND_STR(self->editor->syntax_map, opt_syntax, use_syntax);
    } else if (self->editor->is_in_init && self->editor->syntax_override) {
        // Set by override at init
        HASH_FIND_STR(self->editor->syntax_map, self->editor->syntax_override, use_syntax);
    } else if (self->buffer->path) {
        // Set by path
        for (syntax = self->editor->syntax_last; syntax != NULL; syntax = syntax->hh.prev) {
            if (util_pcre_match(syntax->path_pattern, self->buffer->path, strlen(self->buffer->path), NULL, NULL)) {
                use_syntax = syntax;
                break;
            }
        }
    }

    buffer_set_styles_enabled(self->buffer, 0);

    // Remove current syntax
    if (self->syntax) {
        DL_FOREACH(self->syntax->srules, srule_node) {
            buffer_remove_srule(self->buffer, srule_node->srule);
            self->syntax = NULL;
        }
    }

    // Set syntax if found
    if (use_syntax) {
        DL_FOREACH(use_syntax->srules, srule_node) {
            buffer_add_srule(self->buffer, srule_node->srule);
        }
        self->syntax = use_syntax;
    }

    // Set tab settings
    self->tab_to_space = (use_syntax && use_syntax->tab_to_space >= 0)
        ? use_syntax->tab_to_space
        : self->editor->tab_to_space;
    _bview_set_tab_width(self, (use_syntax && use_syntax->tab_width >= 1)
        ? use_syntax->tab_width
        : self->editor->tab_width
    );

    buffer_set_styles_enabled(self->buffer, 1);

    return use_syntax ? MLE_OK : MLE_ERR;
}

static void _bview_set_tab_width(bview_t *self, int tab_width) {
    self->tab_width = tab_width;
    if (self->buffer && self->buffer->tab_width != self->tab_width) {
        buffer_set_tab_width(self->buffer, self->tab_width);
    }
}

// Attempt to replace leading ~/ with $HOME
static void _bview_expand_tilde(bview_t *self, char *path, int path_len, char **ret_path, int *ret_path_len) {
    char *homedir;
    char *newpath;
    (void)self;
    if (!util_is_file("~", NULL, NULL)
        && strncmp(path, "~/", 2) == 0
        && (homedir = getenv("HOME")) != NULL
    ) {
        newpath = malloc(strlen(homedir) + 1 + (path_len - 2) + 1);
        sprintf(newpath, "%s/%.*s", homedir, path_len-2, path+2);
        *ret_path = newpath;
        *ret_path_len = strlen(*ret_path);
        return;
    }
    *ret_path = strndup(path, path_len);
    *ret_path_len = strlen(*ret_path);
}

// Attempt to fix path by stripping away git-style diff prefixes ([ab/]) and/or
// by extracting a trailing line number after a colon (:)
static void _bview_fix_path(bview_t *self, char *path, int path_len, char **ret_path, int *ret_path_len, bint_t *ret_line_num) {
    char *tmp;
    int tmp_len;
    char *colon;
    int is_valid;
    int fix_nudge;
    int fix_len;
    bint_t line_num;
    (void)self;

    fix_nudge = 0;
    fix_len = path_len;
    line_num = 0;

    // Path already valid?
    if (util_is_file(path, NULL, NULL) || util_is_dir(path)) {
        goto _bview_fix_path_ret;
    }

    // Path valid if we strip "[ab]/" prefix?
    if (path_len >= 3
        && (strncmp(path, "a/", 2) == 0 || strncmp(path, "b/", 2) == 0)
        && (util_is_file(path+2, NULL, NULL) || util_is_dir(path+2))
    ) {
        fix_nudge = 2;
        fix_len -= 2;
        goto _bview_fix_path_ret;
    }

    // Path valid if we extract line num after colon?
    if ((colon = strrchr(path, ':')) != NULL) {
        tmp_len = colon - path;
        tmp = strndup(path, tmp_len);
        is_valid = util_is_file(tmp, NULL, NULL) ? 1 : 0;
        free(tmp);
        if (is_valid) {
            fix_len = tmp_len;
            line_num = strtoul(colon + 1, NULL, 10);
            goto _bview_fix_path_ret;
        }
    }

    // Path valid if we strip "[ab]/" prefix and extract line num?
    if (path_len >= 3
        && (strncmp(path, "a/", 2) == 0 || strncmp(path, "b/", 2) == 0)
        && (colon = strrchr(path, ':')) != NULL
    ) {
        tmp_len = (colon - path) - 2;
        tmp = strndup(path+2, tmp_len);
        is_valid = util_is_file(tmp, NULL, NULL) ? 1 : 0;
        free(tmp);
        if (is_valid) {
            fix_nudge = 2;
            fix_len = tmp_len;
            line_num = strtoul(colon + 1, NULL, 10);
            goto _bview_fix_path_ret;
        }
    }

_bview_fix_path_ret:
    *ret_path = strndup(path + fix_nudge, fix_len);
    *ret_path_len = strlen(*ret_path);
    *ret_line_num = line_num > 0 ? line_num - 1 : 0;
}

// Open a buffer with an optional path to load, otherwise empty
static buffer_t *_bview_open_buffer(bview_t *self, char *opt_path, int opt_path_len) {
    buffer_t *buffer;
    int has_path;
    char *fix_path;
    char *exp_path;
    int fix_path_len;
    int exp_path_len;
    bint_t startup_line_num;

    buffer = NULL;
    has_path = opt_path && opt_path_len > 0 ? 1 : 0;

    if (has_path) {
        _bview_expand_tilde(self, opt_path, opt_path_len, &exp_path, &exp_path_len);
        _bview_fix_path(self, exp_path, exp_path_len, &fix_path, &fix_path_len, &startup_line_num);
        buffer = buffer_new_open(fix_path);
        if (buffer) self->startup_linenum = startup_line_num;
        free(fix_path);
        free(exp_path);
    }
    if (!buffer) {
        buffer = buffer_new();
        if (has_path) {
            buffer->path = strndup(opt_path, opt_path_len);
        }
    }
    buffer_set_callback(buffer, _bview_buffer_callback, self);
    _bview_set_tab_width(self, self->tab_width);
    return buffer;
}

static void _bview_draw_prompt(bview_t *self) {
    _bview_draw_bline(self, self->buffer->first_line, 0, NULL, NULL);
}

static void _bview_draw_status(bview_t *self) {
    editor_t *editor;
    bview_t *active;
    bview_t *active_edit;
    mark_t *mark;

    editor = self->editor;
    active = editor->active;
    active_edit = editor->active_edit;
    mark = active_edit->active_cursor->mark;

    // Prompt
    if (active == editor->prompt) {
        tb_printf(editor->rect_status, 0, 0, TB_GREEN | TB_BOLD, TB_BLACK, "%-*.*s", editor->rect_status.w, editor->rect_status.w, self->editor->prompt->prompt_str);
        goto _bview_draw_status_end;
    }

    // Macro indicator
    int i_macro_fg, i_macro_bg;
    char *i_macro;
    if (editor->is_recording_macro) {
        i_macro_fg = TB_RED | TB_BOLD;
        i_macro_bg = TB_BLACK;
        i_macro = "r";
    } else if (editor->macro_apply) {
        i_macro_fg = TB_GREEN | TB_BOLD;
        i_macro_bg = TB_BLACK;
        i_macro = "p";
    } else {
        i_macro_fg = 0;
        i_macro_bg = 0;
        i_macro = ".";
    }

    // Anchor indicator
    int i_anchor_fg, i_anchor_bg;
    bint_t anchor_len, anchor_nlines;
    char *i_anchor;
    cursor_t *cursor;
    if (active_edit->active_cursor->is_anchored) {
        i_anchor_fg = TB_WHITE | TB_BOLD;
        i_anchor_bg = TB_BLACK;
        i_anchor = "a";
        cursor = active_edit->active_cursor;
        mark_get_nchars_between(cursor->anchor, cursor->mark, &anchor_len);
        if (mark_is_gt(cursor->anchor, cursor->mark)) anchor_len *= -1;
        anchor_nlines = cursor->anchor->bline->line_index - cursor->mark->bline->line_index;
        if (anchor_nlines < 0) anchor_nlines *= -1;
        anchor_nlines += 1;
    } else {
        i_anchor_fg = 0;
        i_anchor_bg = 0;
        i_anchor = ".";
        anchor_len = 0;
        anchor_nlines = 0;
    }

    // Async indicator
    int i_async_fg, i_async_bg;
    char *i_async;
    if (editor->aprocs) {
        i_async_fg = TB_YELLOW | TB_BOLD;
        i_async_bg = TB_BLACK;
        i_async = "x";
    } else {
        i_async_fg = 0;
        i_async_bg = 0;
        i_async = ".";
    }

    // Need-more-input icon
    int i_needinput_fg;
    int i_needinput_bg;
    char *i_needinput;
    if (editor->loop_ctx->need_more_input) {
        i_needinput_fg = TB_BLUE | TB_BOLD;
        i_needinput_bg = TB_BLACK;
        i_needinput = "n";
    } else {
        i_needinput_fg = 0;
        i_needinput_bg = 0;
        i_needinput = ".";
    }

    // Bview num TODO pre-compute this
    bview_t *bview_tmp;
    int bview_count = 0;
    int bview_num = 0;
    CDL_FOREACH2(editor->all_bviews, bview_tmp, all_next) {
        if (!MLE_BVIEW_IS_EDIT(bview_tmp)) continue;
        bview_count += 1;
        if (bview_tmp == active_edit) bview_num = bview_count;
    }

    // Render status line
    MLBUF_BLINE_ENSURE_CHARS(mark->bline);
    tb_printf(editor->rect_status, 0, 0, 0, 0, "%*.*s", editor->rect_status.w, editor->rect_status.w, " ");
    tb_printf_attr(editor->rect_status, 0, 0,
        "@%d,%d;%s@%d,%d;"                                // mle_normal    mode
        "[@%d,%d;%s@%d,%d;%s@%d,%d;%s@%d,%d;%s@%d,%d;]  " // [....]        need_input,anchor,macro,async
        "buf:@%d,%d;%d@%d,%d;/@%d,%d;%d@%d,%d;  "         // buf:1/2       bview num
        "<@%d,%d;%s@%d,%d;>  "                            // <php>         syntax
        "line:@%d,%d;%llu@%d,%d;/@%d,%d;%llu@%d,%d;  "    // line:1/100    line
        "col:@%d,%d;%llu@%d,%d;/@%d,%d;%llu@%d,%d;  "     // col:0/80      col
        "%s@%d,%d;%lld@%d,%d;,@%d,%d;%llu@%d,%d;  ",      // sel:10,1      sel len, nlines
        TB_MAGENTA | TB_BOLD, 0, active->kmap_tail->kmap->name, 0, 0,
        i_needinput_fg, i_needinput_bg, i_needinput,
        i_anchor_fg, i_anchor_bg, i_anchor,
        i_macro_fg, i_macro_bg, i_macro,
        i_async_fg, i_async_bg, i_async, 0, 0,
        TB_BLUE | TB_BOLD, 0, bview_num, 0, 0, TB_BLUE, 0, bview_count, 0, 0,
        TB_CYAN | TB_BOLD, 0, active_edit->syntax ? active_edit->syntax->name : "none", 0, 0,
        TB_YELLOW | TB_BOLD, 0, mark->bline->line_index + 1, 0, 0, TB_YELLOW, 0, active_edit->buffer->line_count, 0, 0,
        TB_YELLOW | TB_BOLD, 0, mark->col, 0, 0, TB_YELLOW, 0, mark->bline->char_count, 0, 0,
        anchor_nlines == 0 ? "" : "sel:",
        anchor_nlines == 0 ? TB_BLACK : TB_YELLOW | TB_BOLD, 0, anchor_len,
        anchor_nlines == 0 ? TB_BLACK : 0, 0,
        anchor_nlines == 0 ? TB_BLACK : TB_YELLOW, 0, anchor_nlines, 0, 0
    );

    // Overlay errstr if present
_bview_draw_status_end:
    if (editor->errstr[0] != '\0') {
        int errstrlen = strlen(editor->errstr) + 5; // Add 5 for "err! "
        tb_printf(editor->rect_status, editor->rect_status.w - errstrlen, 0, TB_WHITE | TB_BOLD, TB_RED, "err! %s", editor->errstr);
        editor->errstr[0] = '\0'; // Clear errstr
    } else if (editor->infostr[0] != '\0') {
        int infostrlen = strlen(editor->infostr);
        tb_printf(editor->rect_status, editor->rect_status.w - infostrlen, 0, TB_WHITE, 0, "%s", editor->infostr);
        editor->infostr[0] = '\0'; // Clear errstr
    }
}

static void _bview_draw_edit(bview_t *self, int x, int y, int w, int h) {
    int split_w;
    int split_h;
    int min_w;
    int min_h;
    int rect_y;
    int fg_attr;
    int bg_attr;
    bline_t *bline;

    // Handle split
    if (self->split_child) {
        // Calc split dimensions
        if (self->split_is_vertical) {
            split_w = w - (int)((float)w * self->split_factor);
            split_h = h;
        } else {
            split_w = w;
            split_h = h - (int)((float)h * self->split_factor);
        }

        // Draw child
        _bview_draw_edit(self->split_child, x + (w - split_w), y + (h - split_h), split_w, split_h);

        // Continue drawing self minus split dimensions
        w -= (w - split_w);
        h -= (h - split_h);
    }

    // Calc min dimensions
    min_w = self->linenum_width + 3;
    min_h = 2;

    // Ensure renderable
    if (w < min_w || h < min_h
        || x + w > self->editor->w
        || y + h > self->editor->h
    ) {
        return;
    }

    // Render caption
    fg_attr = self->editor->active_edit == self ? TB_BOLD : 0;
    bg_attr = self->editor->active_edit == self ? TB_BLUE : 0;
    tb_printf(self->rect_caption, 0, 0, fg_attr, bg_attr, "%*.*s", self->rect_caption.w, self->rect_caption.w, " ");
    if (self->buffer->path) {
        tb_printf(self->rect_caption, 0, 0, fg_attr, bg_attr, "%*.s%s %c",
            self->linenum_width, " ",
            self->buffer->path, self->buffer->is_unsaved ? '*' : ' ');
    } else {
        tb_printf(self->rect_caption, 0, 0, fg_attr, bg_attr, "%*.s<buffer-%p> %c",
            self->linenum_width, " ",
            self->buffer, self->buffer->is_unsaved ? '*' : ' ');
    }

    // Render lines and margins
    if (!self->viewport_bline) {
        buffer_get_bline(self->buffer, MLE_MAX(0, self->viewport_y), &self->viewport_bline);
    }
    bline = self->viewport_bline;
    for (rect_y = 0; rect_y < self->rect_buffer.h; rect_y++) {
        if (self->viewport_y + rect_y < 0 || self->viewport_y + rect_y >= self->buffer->line_count || !bline) { // "|| !bline" See TODOs below
            // Draw pre/post blank
            tb_printf(self->rect_lines, 0, rect_y, 0, 0, "%*c", self->linenum_width, '~');
            tb_printf(self->rect_margin_left, 0, rect_y, 0, 0, "%c", ' ');
            tb_printf(self->rect_margin_right, 0, rect_y, 0, 0, "%c", ' ');
            tb_printf(self->rect_buffer, 0, rect_y, 0, 0, "%-*.*s", self->rect_buffer.w, self->rect_buffer.w, " ");
        } else {
            // Draw bline at self->rect_buffer self->viewport_y + rect_y
            // TODO How can bline be NULL here?
            // TODO How can self->viewport_y != self->viewport_bline->line_index ?
            _bview_draw_bline(self, bline, rect_y, &bline, &rect_y);
            bline = bline->next;
        }
    }
}

static void _bview_draw_bline(bview_t *self, bline_t *bline, int rect_y, bline_t **optret_bline, int *optret_rect_y) {
    int rect_x;
    bint_t char_col;
    int fg;
    int bg, tbg;
    uint32_t ch;
    int char_w;
    bint_t viewport_x;
    bint_t viewport_x_vcol;
    int i, j;
    int is_cursor_line;
    int is_soft_wrap;
    int orig_rect_y;
    srule_t *range_srule;

    MLBUF_BLINE_ENSURE_CHARS(bline);

    // Set is_cursor_line
    is_cursor_line = self->active_cursor->mark->bline == bline ? 1 : 0;

    // Soft wrap only for current line
    is_soft_wrap = self->soft_wrap && is_cursor_line && MLE_BVIEW_IS_EDIT(self) ? 1 : 0;

    // Use viewport_x only for current line when not soft wrapping
    viewport_x = 0;
    viewport_x_vcol = 0;
    if (is_cursor_line && !is_soft_wrap) {
        viewport_x = self->viewport_x;
        viewport_x_vcol = self->viewport_x_vcol;
    }

    // Draw linenums and margins
    if (MLE_BVIEW_IS_EDIT(self)) {
        int linenum_fg = is_cursor_line ? TB_BOLD : 0;
        if (self->editor->linenum_type == MLE_LINENUM_TYPE_ABS
            || self->editor->linenum_type == MLE_LINENUM_TYPE_BOTH
            || (self->editor->linenum_type == MLE_LINENUM_TYPE_REL && is_cursor_line)
        ) {
            tb_printf(self->rect_lines, 0, rect_y, linenum_fg, 0, "%*d", self->abs_linenum_width, (int)(bline->line_index + 1) % (int)pow(10, self->linenum_width));
            if (self->editor->linenum_type == MLE_LINENUM_TYPE_BOTH) {
                tb_printf(self->rect_lines, self->abs_linenum_width, rect_y, linenum_fg, 0, " %*d", self->rel_linenum_width, (int)labs(bline->line_index - self->active_cursor->mark->bline->line_index));
            }
        } else if (self->editor->linenum_type == MLE_LINENUM_TYPE_REL) {
            tb_printf(self->rect_lines, 0, rect_y, linenum_fg, 0, "%*d", self->rel_linenum_width, (int)labs(bline->line_index - self->active_cursor->mark->bline->line_index));
        }
        tb_printf(self->rect_margin_left, 0, rect_y, 0, 0, "%c", viewport_x > 0 && bline->char_count > 0 ? '^' : ' ');
        if (!is_soft_wrap && bline->char_vwidth - viewport_x_vcol > self->rect_buffer.w) {
            tb_printf(self->rect_margin_right, 0, rect_y, 0, 0, "%c", '$');
        }
    }

    // Render 0 thru rect_buffer.w cell by cell
    orig_rect_y = rect_y;
    rect_x = 0;
    char_col = viewport_x;
    while (char_col < bline->char_count) {
        ch = bline->chars[char_col].ch;
        fg = bline->chars[char_col].style.fg;
        bg = bline->chars[char_col].style.bg;
        char_w = char_col == bline->char_count - 1
            ? bline->char_vwidth - bline->chars[char_col].vcol
            : bline->chars[char_col + 1].vcol - bline->chars[char_col].vcol;
        if (ch == '\t') {
            ch = ' ';
        } else if (!iswprint(ch)) {
            ch = '?';
        }
        if (MLE_BVIEW_IS_MENU(self) && is_cursor_line) {
            // Highlight menu line
            bg |= TB_REVERSE;
        }
        if (_bview_is_in_range(bline, char_col, &range_srule)) {
            // Highlight range
            fg = range_srule->style.fg;
            bg = range_srule->style.bg;
        }
        // Draw char_w chars of ch
        for (i = 0; i < char_w && rect_x + i < self->rect_buffer.w; i++) {
            if (self->editor->color_col == rect_x + i && MLE_BVIEW_IS_EDIT(self)) {
                // Apply color col style
                tbg = bg | TB_RED;
            } else {
                tbg = bg;
            }
            tb_change_cell(self->rect_buffer.x + rect_x + i, self->rect_buffer.y + rect_y, ch, fg, tbg);
        }
        if (i < char_w) {
            // There was not enough width to draw
            if (is_soft_wrap && rect_y + 1 < self->rect_buffer.h) {
                // Soft wrap to next line
                rect_x = 0;
                rect_y += 1;
                char_w -= i;
                // Draw remaining ch on next line
                for (j = 0; j < char_w && rect_x + j < self->rect_buffer.w; j++) {
                    tb_change_cell(self->rect_buffer.x + j, self->rect_buffer.y + rect_y, ch, fg, bg);
                }
                // Draw ellipsis for line num to indicate soft wrap
                for (j = 0; j < self->linenum_width; j++) {
                    tb_printf(self->rect_lines, j, rect_y, 0, 0, "%c", '.');
                }
            } else {
                // We are off-screen and soft-wrap is not enabled or there is not
                // enough height to soft wrap
                break;
            }
        }
        rect_x += char_w;
        char_col += 1;
    }
    for (i = orig_rect_y; i < rect_y && bline->next; i++) {
        bline = bline->next;
    }
    if (optret_bline) *optret_bline = bline;
    if (optret_rect_y) *optret_rect_y = rect_y;
}

// Highlight matching bracket pair under mark
static void _bview_highlight_bracket_pair(bview_t *self, mark_t *mark) {
    bline_t *line;
    bint_t brkt;
    bint_t col;
    mark_t pair;
    int screen_x;
    int screen_y;
    struct tb_cell *cell;

    MLBUF_BLINE_ENSURE_CHARS(mark->bline);

    if (mark_is_at_eol(mark) || !util_get_bracket_pair(mark->bline->chars[mark->col].ch, NULL)) {
        // Not a bracket
        return;
    }
    if (mark_find_bracket_pair(mark, MLE_BRACKET_PAIR_MAX_SEARCH, &line, &col, &brkt) != MLBUF_OK) {
        // No pair found
        return;
    }
    if (mark->bline == line && (mark->col == col - 1 || mark->col == col + 1)) {
        // One char away, do not highlight (looks confusing in UI)
        return;
    }

    pair.bline = line;
    pair.col = col;
    if (bview_get_screen_coords(self, &pair, &screen_x, &screen_y, &cell) != MLE_OK) {
        // Out of bounds
        return;
    }
    tb_change_cell(screen_x, screen_y, cell->ch, cell->fg | TB_UNDERLINE, cell->bg); // TODO configurable
}

// Find screen coordinates for a mark
int bview_get_screen_coords(bview_t *self, mark_t *mark, int *ret_x, int *ret_y, struct tb_cell **optret_cell) {
    int screen_x;
    int screen_y;
    int is_soft_wrapped;

    MLBUF_BLINE_ENSURE_CHARS(mark->bline);

    is_soft_wrapped = self->soft_wrap
        && self->active_cursor->mark->bline == mark->bline
        && MLE_BVIEW_IS_EDIT(self) ? 1 : 0;

    if (is_soft_wrapped) {
        screen_x = self->rect_buffer.x + MLE_MARK_COL_TO_VCOL(mark) % self->rect_buffer.w;
        screen_y = self->rect_buffer.y + (mark->bline->line_index - self->viewport_bline->line_index) + (MLE_MARK_COL_TO_VCOL(mark) / self->rect_buffer.w);
    } else {
        screen_x = self->rect_buffer.x + MLE_MARK_COL_TO_VCOL(mark) - MLE_COL_TO_VCOL(mark->bline, self->viewport_x, mark->bline->char_vwidth);
        screen_y = self->rect_buffer.y + (mark->bline->line_index - self->viewport_bline->line_index);
    }
    if (screen_x < self->rect_buffer.x || screen_x >= self->rect_buffer.x + self->rect_buffer.w
       || screen_y < self->rect_buffer.y || screen_y >= self->rect_buffer.y + self->rect_buffer.h
    ) {
        // Out of bounds
        return MLE_ERR;
    }
    *ret_x = screen_x;
    *ret_y = screen_y;
    if (optret_cell) {
        *optret_cell = tb_cell_buffer() + (ptrdiff_t)(tb_width() * screen_y + screen_x);
    }
    return MLE_OK;
}

static int _bview_is_in_range(bline_t *bline, bint_t col, srule_t **ret_srule) {
    mark_t mark = {0};
    srule_node_t *node;
    mark.bline = bline;
    mark.col = col;
    DL_FOREACH(bline->buffer->range_srules, node) {
        if (mark_is_between(&mark, node->srule->range_a, node->srule->range_b)) {
            *ret_srule = node->srule;
            return 1;
        }
    }
    return 0;
}
