#include <math.h>
#include <wctype.h>
#include "mle.h"

static void _bview_init(bview_t* self, buffer_t* buffer);
static kmap_t* _bview_get_init_kmap(editor_t* editor);
static void _bview_deinit(bview_t* self);
static buffer_t* _bview_open_buffer(bview_t* self, char* path, int path_len);
static void _bview_draw_prompt(bview_t* self);
static void _bview_draw_status(bview_t* self);
static void _bview_draw_edit(bview_t* self, int x, int y, int w, int h);
static void _bview_draw_bline(bview_t* self, bline_t* bline, int rect_y);
static void _bview_set_syntax(bview_t* self);
static void _bview_buffer_callback(buffer_t* buffer, baction_t* action, void* udata);
static int _bview_rectify_viewport_dim(bview_t* self, bline_t* bline, bint_t vpos, int dim_scope, int dim_size, bint_t *view_vpos);
static bint_t _bview_get_col_from_vcol(bview_t* self, bline_t* bline, bint_t vcol);
static int _bview_set_linenum_width(bview_t* self);
static void _bview_highlight_bracket_pair(bview_t* self, mark_t* mark);
static int _bview_get_screen_coords(bview_t* self, mark_t* mark, int* ret_x, int* ret_y, struct tb_cell** optret_cell);

// Create a new bview
bview_t* bview_new(editor_t* editor, char* opt_path, int opt_path_len, buffer_t* opt_buffer) {
    bview_t* self;
    buffer_t* buffer;

    // Allocate and init bview
    self = calloc(1, sizeof(bview_t));
    self->editor = editor;
    self->path = strndup(opt_path, opt_path_len);
    self->rect_caption.fg = TB_WHITE;
    self->rect_caption.bg = TB_BLACK;
    self->rect_lines.fg = TB_YELLOW;
    self->rect_lines.bg = TB_BLACK;
    self->rect_margin_left.fg = TB_RED;
    self->rect_margin_right.fg = TB_RED;
    self->rect_buffer.h = 10; // TODO hack to fix _bview_set_linenum_width before bview_resize
    self->tab_to_space = editor->tab_to_space;
    self->viewport_scope_x = editor->viewport_scope_x;
    self->viewport_scope_y = editor->viewport_scope_y;

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
int bview_open(bview_t* self, char* path, int path_len) {
    buffer_t* buffer;
    buffer = _bview_open_buffer(self, path, path_len);
    if (self->path) free(self->path);
    self->path = strndup(path, path_len);
    _bview_init(self, buffer);
    return MLE_OK;
}

// Free a bview
int bview_destroy(bview_t* self) {
    _bview_deinit(self);
    if (self->path) free(self->path);
    // TODO ensure everything freed
    free(self);
    return MLE_OK;
}

// Move and resize a bview to the given position and dimensions
int bview_resize(bview_t* self, int x, int y, int w, int h) {
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
        self->rect_buffer.w = aw - (self->linenum_width + 1 + 1);
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

    bview_rectify_viewport(self);

    return MLE_OK;
}

// Return top-most split_parent of a bview
bview_t* bview_get_split_root(bview_t* self) {
    bview_t* root;
    root = self;
    while (root->split_parent) {
        root = root->split_parent;
    }
    return root;
}

// Draw bview to screen
int bview_draw(bview_t* self) {
    if (MLE_BVIEW_IS_PROMPT(self)) {
        _bview_draw_prompt(self);
    } else if (MLE_BVIEW_IS_STATUS(self)) {
        _bview_draw_status(self);
    }
    _bview_draw_edit(self, self->x, self->y, self->w, self->h);
    return MLE_OK;
}

// Set cursor to screen
int bview_draw_cursor(bview_t* self, int set_real_cursor) {
    cursor_t* cursor;
    mark_t* mark;
    int screen_x;
    int screen_y;
    struct tb_cell* cell;
    DL_FOREACH(self->cursors, cursor) {
        mark = cursor->mark;
        if (_bview_get_screen_coords(self, mark, &screen_x, &screen_y, &cell) != MLE_OK) {
            // Out of bounds
            return MLE_OK;
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
int bview_push_kmap(bview_t* bview, kmap_t* kmap) {
    kmap_node_t* node;
    node = calloc(1, sizeof(kmap_node_t));
    node->kmap = kmap;
    node->bview = bview;
    DL_APPEND(bview->kmap_stack, node);
    bview->kmap_tail = node;
    return MLE_OK;
}

// Pop a kmap
int bview_pop_kmap(bview_t* bview, kmap_t** optret_kmap) {
    kmap_node_t* node_to_pop;
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
int bview_split(bview_t* self, int is_vertical, float factor, bview_t** optret_bview) {
    bview_t* child;

    if (self->split_child) {
        MLE_RETURN_ERR("bview %p is already split\n", self);
    } else if (!MLE_BVIEW_IS_EDIT(self)) {
        MLE_RETURN_ERR("bview %p is not an edit bview\n", self);
    }

    // Make child
    editor_open_bview(self->editor, self, self->type, NULL, 0, 1, NULL, self->buffer, &child);
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

// Add a cursor to a bview
int bview_add_cursor(bview_t* self, bline_t* bline, bint_t col, cursor_t** optret_cursor) {
    cursor_t* cursor;
    cursor = calloc(1, sizeof(cursor_t));
    cursor->bview = self;
    cursor->mark = buffer_add_mark(self->buffer, bline, col);
    DL_APPEND(self->cursors, cursor);
    if (!self->active_cursor) {
        self->active_cursor = cursor;
    }
    if (optret_cursor) {
        *optret_cursor = cursor;
    }
    return MLE_OK;
}

// Remove a cursor from a bview
int bview_remove_cursor(bview_t* self, cursor_t* cursor) {
    cursor_t* el;
    cursor_t* tmp;
    DL_FOREACH_SAFE(self->cursors, el, tmp) {
        if (el == cursor) {
            self->active_cursor = el->prev && el->prev != el ? el->prev : el->next;
            DL_DELETE(self->cursors, el);
            if (el->sel_rule) {
                buffer_remove_srule(el->bview->buffer, el->sel_rule);
                srule_destroy(el->sel_rule);
                el->sel_rule = NULL;
            }
            free(el);
            return MLE_OK;
        }
    }
    return MLE_ERR;
}

// Center the viewport vertically
int bview_center_viewport_y(bview_t* self) {
    bint_t center;
    center = self->active_cursor->mark->bline->line_index - self->rect_buffer.h/2;
    if (center < 0) {
        center = 0;
    }
    self->viewport_y = center;
    bview_rectify_viewport(self);
    buffer_get_bline(self->buffer, self->viewport_y, &self->viewport_bline);
    return MLE_OK;
}

// Zero the viewport vertically
int bview_zero_viewport_y(bview_t* self) {
    self->viewport_y = self->active_cursor->mark->bline->line_index;
    bview_rectify_viewport(self);
    buffer_get_bline(self->buffer, self->viewport_y, &self->viewport_bline);
    return MLE_OK;
}

// Rectify the viewport
int bview_rectify_viewport(bview_t* self) {
    mark_t* mark;
    mark = self->active_cursor->mark;

    // Rectify each dimension of the viewport
    _bview_rectify_viewport_dim(self, mark->bline, MLE_MARK_COL_TO_VCOL(mark), self->viewport_scope_x, self->rect_buffer.w, &self->viewport_x_vcol);
    self->viewport_x = _bview_get_col_from_vcol(self, mark->bline, self->viewport_x_vcol);

    if (_bview_rectify_viewport_dim(self, mark->bline, mark->bline->line_index, self->viewport_scope_y, self->rect_buffer.h, &self->viewport_y)) {
        // TODO viewport_y_vrow (soft-wrapped lines, code folding, etc)
        // Refresh viewport_bline
        buffer_get_bline(self->buffer, self->viewport_y, &self->viewport_bline);
    }

    return MLE_OK;
}

// Add a listener
int bview_add_listener(bview_t* self, bview_listener_cb_t callback, void* udata) {
    bview_listener_t* listener;
    listener = calloc(1, sizeof(bview_listener_t));
    listener->callback = callback;
    listener->udata = udata;
    DL_APPEND(self->listeners, listener);
    return MLE_OK;
}

// Remove and free a listener
int bview_destroy_listener(bview_t* self, bview_listener_t* listener) {
    DL_APPEND(self->listeners, listener);
    free(listener);
    return MLE_OK;
}

// Rectify a viewport dimension. Return 1 if changed, else 0.
static int _bview_rectify_viewport_dim(bview_t* self, bline_t* bline, bint_t vpos, int dim_scope, int dim_size, bint_t *view_vpos) {
    int rc;
    bint_t vpos_start;
    bint_t vpos_stop;

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

// Convert a vcol to a col
static bint_t _bview_get_col_from_vcol(bview_t* self, bline_t* bline, bint_t vcol) {
    bint_t i;
    for (i = 0; i < bline->char_count; i++) {
        if (vcol <= bline->chars[i].vcol) {
            return i;
        }
    }
    return bline->char_count;
}

// Init a bview with a buffer
static void _bview_init(bview_t* self, buffer_t* buffer) {
    cursor_t* cursor_tmp;

    _bview_deinit(self);

    // Reference buffer
    self->buffer = buffer;
    self->buffer->ref_count += 1;
    _bview_set_linenum_width(self);

    // Push normal mode
    bview_push_kmap(self, _bview_get_init_kmap(self->editor));

    // Set syntax
    _bview_set_syntax(self);

    // Add a cursor
    bview_add_cursor(self, self->buffer->first_line, 0, &cursor_tmp);
}

// Return initial kmap to use
static kmap_t* _bview_get_init_kmap(editor_t* editor) {
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
static void _bview_buffer_callback(buffer_t* buffer, baction_t* action, void* udata) {
    editor_t* editor;
    bview_t* self;
    bview_t* active;
    bview_listener_t* listener;

    self = (bview_t*)udata;
    editor = self->editor;
    active = editor->active;

    // Rectify viewport if edit was on active bview
    if (active->buffer == buffer) {
        bview_rectify_viewport(active);
    }

    if (action && action->line_delta != 0) {
        bview_t* bview;
        bview_t* tmp1;
        bview_t* tmp2;
        CDL_FOREACH_SAFE2(editor->all_bviews, bview, tmp1, tmp2, all_prev, all_next) {
            if (bview->buffer == buffer) {
                // Adjust linenum_width
                if (_bview_set_linenum_width(bview)) {
                    bview_resize(bview, bview->x, bview->y, bview->w, bview->h);
                }
                // Adjust viewport_bline
                buffer_get_bline(bview->buffer, bview->viewport_y, &bview->viewport_bline);
            }
        }
    }

    // Call bview listeners
    DL_FOREACH(self->listeners, listener) {
        listener->callback(self, action, listener->udata);
    }
}

// Set linenum_width and return 1 if changed
static int _bview_set_linenum_width(bview_t* self) {
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
static void _bview_deinit(bview_t* self) {
    bview_listener_t* listener;
    bview_listener_t* listener_tmp;

    // Remove all kmaps
    while (self->kmap_tail) {
        bview_pop_kmap(self, NULL);
    }

    // Remove all syntax rules
    if (self->syntax) {
        srule_node_t* srule_node;
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
    if (self->async_proc) {
        async_proc_destroy(self->async_proc);
        self->async_proc = NULL;
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
static void _bview_set_syntax(bview_t* self) {
    syntax_t* syntax;
    syntax_t* syntax_tmp;
    syntax_t* use_syntax;
    srule_node_t* srule_node;

    // Only set syntax on edit bviews
    if (!MLE_BVIEW_IS_EDIT(self)) {
        return;
    }

    use_syntax = NULL;
    if (self->editor->is_in_init && self->editor->syntax_override) {
        // Set by override
        HASH_FIND_STR(self->editor->syntax_map, self->editor->syntax_override, use_syntax);
    } else if (self->buffer->path) {
        // Set by path
        HASH_ITER(hh, self->editor->syntax_map, syntax, syntax_tmp) {
            if (util_pcre_match(self->buffer->path, syntax->path_pattern)) {
                use_syntax = syntax;
                break;
            }
        }
    }

    // Set syntax if found
    if (use_syntax) {
        buffer_set_styles_enabled(self->buffer, 0);
        DL_FOREACH(use_syntax->srules, srule_node) {
            buffer_add_srule(self->buffer, srule_node->srule);
        }
        buffer_set_styles_enabled(self->buffer, 1);
        self->syntax = use_syntax;
    }
}

// Open a buffer with an optional path to load, otherwise empty
static buffer_t* _bview_open_buffer(bview_t* self, char* opt_path, int opt_path_len) {
    buffer_t* buffer;
    buffer = NULL;
    if (opt_path && opt_path_len > 0) {
        buffer = buffer_new_open(opt_path, opt_path_len);
    }
    if (!buffer) {
        buffer = buffer_new();
    }
    buffer_set_callback(buffer, _bview_buffer_callback, self);
    if (buffer->tab_width != self->editor->tab_width) {
        buffer_set_tab_width(buffer, self->editor->tab_width);
    }
    return buffer;
}

static void _bview_draw_prompt(bview_t* self) {
    _bview_draw_bline(self, self->buffer->first_line, 0);
}

static void _bview_draw_status(bview_t* self) {
    editor_t* editor;
    bview_t* active;
    bview_t* active_edit;
    mark_t* mark;

    editor = self->editor;
    active = editor->active;
    active_edit = editor->active_edit;
    mark = active_edit->active_cursor->mark;

    // Prompt
    if (active == editor->prompt) {
        tb_printf(editor->rect_status, 0, 0, TB_GREEN | TB_BOLD, TB_BLACK, "%-*.*s", editor->rect_status.w, editor->rect_status.w, self->editor->prompt->prompt_str);
        return;
    }

    // Macro indicator
    int i_macro_fg, i_macro_bg;
    char* i_macro;
    if (editor->is_recording_macro) {
        i_macro_fg = TB_RED | TB_BOLD;
        i_macro_bg = TB_WHITE;
        i_macro = "\xe2\x97\x8f";
    } else if (editor->macro_apply) {
        i_macro_fg = TB_WHITE | TB_BOLD;
        i_macro_bg = TB_GREEN;
        i_macro = "\xe2\x96\xb6";
    } else {
        i_macro_fg = 0;
        i_macro_bg = 0;
        i_macro = ".";
    }

    // Async indicator
    int i_async_fg, i_async_bg;
    char* i_async;
    static int i_async_idx = 0;
    if (editor->async_procs) {
        i_async_fg = TB_BLACK | TB_BOLD;
        i_async_bg = TB_YELLOW;
        if (i_async_idx < 100)      i_async = "\xe2\x96\x9d";
        else if (i_async_idx < 200) i_async = "\xe2\x96\x97";
        else if (i_async_idx < 300) i_async = "\xe2\x96\x96";
        else if (i_async_idx < 400) i_async = "\xe2\x96\x98";
        if (++i_async_idx >= 400) i_async_idx = 0;
    } else {
        i_async_fg = 0;
        i_async_bg = 0;
        i_async = ".";
    }

    // Need-more-input icon
    int i_needinput_fg;
    int i_needinput_bg;
    char* i_needinput;
    if (editor->loop_ctx->need_more_input) {
        i_needinput_fg = TB_BLACK;
        i_needinput_bg = TB_BLUE;
        i_needinput = "\xe2\x80\xa6";
    } else {
        i_needinput_fg = 0;
        i_needinput_bg = 0;
        i_needinput = ".";
    }

    // Bview num .. TODO pre-compute this
    bview_t* bview_tmp;
    int bview_count = 0;
    int bview_num = 0;
    CDL_FOREACH2(editor->all_bviews, bview_tmp, all_next) {
        if (MLE_BVIEW_IS_EDIT(bview_tmp)) {
            bview_count += 1;
            if (bview_tmp == active_edit) bview_num = bview_count;
        }
    }

    // Render status line
    tb_printf(self->editor->rect_status, 0, 0, 0, 0, "%*.*s", self->editor->rect_status.w, self->editor->rect_status.w, " ");
    tb_printf_attr(self->editor->rect_status, 0, 0,
        "@%d,%d;%s@%d,%d;"                             // mle_normal    mode
        "(@%d,%d;%s@%d,%d;%s@%d,%d;%s@%d,%d;)  "       // (...)         need_input,macro,async
        "buf:@%d,%d;%d@%d,%d;/@%d,%d;%d@%d,%d;  "     // buf[1/2]      bview num
        "<@%d,%d;%s@%d,%d;>  "                         // <php>         syntax
        "line:@%d,%d;%llu@%d,%d;/@%d,%d;%llu@%d,%d;  " // line:1/100    line
        "col:@%d,%d;%llu@%d,%d;/@%d,%d;%llu@%d,%d; ",  // col:0/80      col
        TB_MAGENTA | TB_BOLD, 0, active->kmap_tail->kmap->name, 0, 0,
        i_needinput_fg, i_needinput_bg, i_needinput,
        i_macro_fg, i_macro_bg, i_macro,
        i_async_fg, i_async_bg, i_async, 0, 0,
        TB_BLUE | TB_BOLD, 0, bview_num, 0, 0, TB_BLUE, 0, bview_count, 0, 0,
        TB_CYAN | TB_BOLD, 0, active_edit->syntax ? active_edit->syntax->name : "none", 0, 0,
        TB_YELLOW | TB_BOLD, 0, mark->bline->line_index + 1, 0, 0, TB_YELLOW, 0, active_edit->buffer->line_count, 0, 0,
        TB_YELLOW | TB_BOLD, 0, mark->col, 0, 0, TB_YELLOW, 0, mark->bline->char_count, 0, 0
    );
}

static void _bview_draw_edit(bview_t* self, int x, int y, int w, int h) {
    int split_w;
    int split_h;
    int min_w;
    int min_h;
    int rect_y;
    int fg_attr;
    int bg_attr;
    bline_t* bline;

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
            _bview_draw_bline(self, bline, rect_y);
            bline = bline->next;
        }
    }
}

static void _bview_draw_bline(bview_t* self, bline_t* bline, int rect_y) {
    int rect_x;
    bint_t char_col;
    int fg;
    int bg;
    uint32_t ch;
    int char_w;
    bint_t viewport_x;
    bint_t viewport_x_vcol;
    int i;
    int is_cursor_line;

    // Use viewport_x only for current line
    viewport_x = 0;
    viewport_x_vcol = 0;
    is_cursor_line = self->active_cursor->mark->bline == bline ? 1 : 0;
    if (is_cursor_line) {
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
                tb_printf(self->rect_lines, self->abs_linenum_width, rect_y, linenum_fg, 0, " %*d", self->rel_linenum_width, (int)abs(bline->line_index - self->active_cursor->mark->bline->line_index));
            }
        } else if (self->editor->linenum_type == MLE_LINENUM_TYPE_REL) {
            tb_printf(self->rect_lines, 0, rect_y, linenum_fg, 0, "%*d", self->rel_linenum_width, (int)abs(bline->line_index - self->active_cursor->mark->bline->line_index));
        }
        tb_printf(self->rect_margin_left, 0, rect_y, 0, 0, "%c", viewport_x > 0 && bline->char_count > 0 ? '^' : ' ');
        tb_printf(self->rect_margin_right, 0, rect_y, 0, 0, "%c", bline->char_vwidth - viewport_x_vcol > self->rect_buffer.w ? '$' : ' ');
    }

    // Render 0 thru rect_buffer.w cell by cell
    for (rect_x = 0, char_col = viewport_x; rect_x < self->rect_buffer.w; char_col++) {
        char_w = 1;
        if (char_col < bline->char_count) {
            ch = bline->chars[char_col].ch;
            fg = bline->char_styles[char_col].fg;
            bg = bline->char_styles[char_col].bg;
            char_w = char_col == bline->char_count - 1
                ? bline->char_vwidth - bline->chars[char_col].vcol
                : bline->chars[char_col + 1].vcol - bline->chars[char_col].vcol;
            if (ch == '\t') {
                ch = ' ';
            } else if (!iswprint(ch)) {
                ch = '?';
            }
            if (self->editor->color_col == char_col && MLE_BVIEW_IS_EDIT(self)) {
                bg |= TB_RED;
            }
        } else if (char_col == bline->char_count && bline->eol_rule) {
            ch = (uint32_t)' ';
            fg = bline->eol_rule->style.fg;
            bg = bline->eol_rule->style.bg;
            char_w = 1;
        } else {
            ch = (uint32_t)' ';
            fg = 0;
            bg = 0;
            char_w = 1;
        }
        if (MLE_BVIEW_IS_MENU(self) && is_cursor_line) {
            bg |= TB_REVERSE;
        }
        for (i = 0; i < char_w && rect_x < self->rect_buffer.w; i++) {
            tb_change_cell(self->rect_buffer.x + rect_x + i, self->rect_buffer.y + rect_y, ch, fg, bg);
        }
        rect_x += char_w;
    }
}

// Highlight matching bracket pair under mark
static void _bview_highlight_bracket_pair(bview_t* self, mark_t* mark) {
    bline_t* line;
    bint_t brkt;
    bint_t col;
    mark_t pair;
    int screen_x;
    int screen_y;
    struct tb_cell* cell;
    if (mark_is_at_eol(mark) || !util_get_bracket_pair(mark->bline->chars[mark->col].ch, NULL)) {
        // Not a bracket
        return;
    }
    if (mark_find_bracket_pair(mark, MLE_BRACKET_PAIR_MAX_SEARCH, &line, &col, &brkt) != MLBUF_OK) {
        // No pair found
        return;
    }
    pair.bline = line;
    pair.col = col;
    if (_bview_get_screen_coords(self, &pair, &screen_x, &screen_y, &cell) != MLE_OK) {
        // Out of bounds
        return;
    }
    tb_change_cell(screen_x, screen_y, cell->ch, cell->fg, cell->bg | TB_REVERSE); // TODO configurable
}

// Find screen coordinates for a mark
static int _bview_get_screen_coords(bview_t* self, mark_t* mark, int* ret_x, int* ret_y, struct tb_cell** optret_cell) {
    int screen_x;
    int screen_y;
    screen_x = self->rect_buffer.x + MLE_MARK_COL_TO_VCOL(mark) - MLE_COL_TO_VCOL(mark->bline, self->viewport_x, mark->bline->char_vwidth);
    screen_y = self->rect_buffer.y + (mark->bline->line_index - self->viewport_bline->line_index);
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
