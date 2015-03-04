#include <math.h>
#include <wctype.h>
#include "mle.h"

static void _bview_init(bview_t* self, buffer_t* buffer);
static void _bview_deinit(bview_t* self);
static buffer_t* _bview_open_buffer(char* path, int path_len, editor_t* editor);
static void _bview_draw_prompt(bview_t* self);
static void _bview_draw_popup(bview_t* self);
static void _bview_draw_status(bview_t* self);
static void _bview_draw_edit(bview_t* self, int x, int y, int w, int h);
static void _bview_draw_bline(bview_t* self, bline_t* bline, int rect_y);
static void _bview_set_syntax(bview_t* self);
static void _bview_buffer_callback(buffer_t* buffer, baction_t* action, void* udata);
static int _bview_rectify_viewport_dim(bview_t* self, bline_t* bline, bint_t vpos, int dim_scope, int dim_size, bint_t *view_vpos);
static bint_t _bview_get_col_from_vcol(bview_t* self, bline_t* bline, bint_t vcol);
static int _bview_set_line_num_width(bview_t* self);

// Create a new bview
bview_t* bview_new(editor_t* editor, char* opt_path, int opt_path_len, buffer_t* opt_buffer) {
    bview_t* self;
    buffer_t* buffer;

    // Open buffer
    if (opt_buffer) {
        buffer = opt_buffer;
    } else {
        buffer = _bview_open_buffer(opt_path, opt_path_len, editor);
    }

    // Allocate and init bview
    self = calloc(1, sizeof(bview_t));
    self->editor = editor;
    self->path = strndup(opt_path, opt_path_len);
    self->rect_caption.bg = TB_REVERSE; // TODO configurable
    self->rect_lines.fg = TB_YELLOW;
    self->rect_margin_left.fg = TB_RED;
    self->rect_margin_right.fg = TB_RED;
    self->tab_to_space = editor->tab_to_space;
    self->viewport_scope_x = editor->viewport_scope_x;
    self->viewport_scope_y = editor->viewport_scope_y;
    _bview_init(self, buffer);

    return self;
}

// Open a buffer in an existing bview
int bview_open(bview_t* self, char* path, int path_len) {
    buffer_t* buffer;
    buffer = _bview_open_buffer(path, path_len, self->editor);
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
        self->rect_lines.w = self->line_num_width;
        self->rect_lines.h = ah - 1;

        self->rect_margin_left.x = x + self->line_num_width;
        self->rect_margin_left.y = y + 1;
        self->rect_margin_left.w = 1;
        self->rect_margin_left.h = ah - 1;

        self->rect_buffer.x = x + self->line_num_width + 1;
        self->rect_buffer.y = y + 1;
        self->rect_buffer.w = aw - (self->line_num_width + 1 + 1);
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

// Draw bview to screen
int bview_draw(bview_t* self) {
    if (MLE_BVIEW_IS_PROMPT(self)) {
        _bview_draw_prompt(self);
    } else if (MLE_BVIEW_IS_POPUP(self)) {
        _bview_draw_popup(self);
    } else if (MLE_BVIEW_IS_STATUS(self)) {
        _bview_draw_status(self);
    }
    _bview_draw_edit(self, self->x, self->y, self->w, self->h);
    return MLE_OK;
}

// Set cursor to screen
int bview_draw_cursor(bview_t* self) {
    mark_t* mark;
    int rect_x;
    int rect_y;
    mark = self->active_cursor->mark;
    rect_x = MLE_MARK_COL_TO_VCOL(mark) - MLE_COL_TO_VCOL(mark->bline, self->viewport_x, mark->bline->char_vwidth);
    rect_y = mark->bline->line_index - self->viewport_y;
    tb_set_cursor(self->rect_buffer.x + rect_x, self->rect_buffer.y + rect_y);
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
    editor_open_bview(self->editor, self->type, NULL, 0, 1, self->buffer, &child);
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
        if (vcol <= bline->char_vcol[i]) {
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
    _bview_set_line_num_width(self);

    // Push normal mode
    bview_push_kmap(self, self->editor->kmap_normal);

    // Set syntax
    _bview_set_syntax(self);

    // Add a cursor
    bview_add_cursor(self, self->buffer->first_line, 0, &cursor_tmp);
}

// Called by mlbuf after edits
static void _bview_buffer_callback(buffer_t* buffer, baction_t* action, void* udata) {
    editor_t* editor;
    bview_t* bview;
    bview_t* bview_tmp;

    editor = (editor_t*)udata;
    bview = editor->active;

    // Rectify viewport if edit was on active bview
    if (bview->buffer == buffer) {
        bview_rectify_viewport(bview);
    }

    #define BVIEW_ITERATE_FOR_BUFFER(tmp) \
        for (tmp = editor->bviews; tmp; tmp = bview_tmp->next) \
            if (tmp->buffer == buffer)

    // Adjust line_num_width
    if (action && action->line_delta != 0) {
        BVIEW_ITERATE_FOR_BUFFER(bview_tmp) {
            if (_bview_set_line_num_width(bview)) {
                bview_resize(bview_tmp, bview_tmp->x, bview_tmp->y, bview_tmp->w, bview_tmp->h);
            }
        }
    }

    #undef BVIEW_ITERATE_FOR_BUFFER
}

// Set line_num_width and return 1 if changed
static int _bview_set_line_num_width(bview_t* self) {
    int orig;
    orig = self->line_num_width;
    self->line_num_width = MLE_MAX(1, (int)(floor(log10((double)self->buffer->line_count))) + 1);
    return orig == self->line_num_width ? 0 : 1;
}

// Deinit a bview
static void _bview_deinit(bview_t* self) {
    // Remove all kmaps
    while (self->kmap_tail) {
        bview_pop_kmap(self, NULL);
    }

    // Remove all syntax rules
    if (self->syntax) {
        srule_node_t* srule_node;
        DL_FOREACH(self->syntax->srules, srule_node) {
            buffer_remove_srule(self->buffer, srule_node->srule);
        }
    }

    // Remove all cursors
    while (self->active_cursor) {
        bview_remove_cursor(self, self->active_cursor);
    }

    // Dereference/free buffer
    if (self->buffer) {
        self->buffer->ref_count -= 1;
        if (self->buffer->ref_count < 1) {
            buffer_destroy(self->buffer);
        }
    }
}

static void _bview_set_syntax(bview_t* self) {
    // TODO set by self->buffer->path pattern
    syntax_t* syntax;
    syntax_t* syntax_tmp;
    srule_node_t* srule_node;
    if (!MLE_BVIEW_IS_EDIT(self)) {
        return;
    }
    HASH_ITER(hh, self->editor->syntax_map, syntax, syntax_tmp) {
        self->syntax = syntax;
        DL_FOREACH(syntax->srules, srule_node) {
            buffer_add_srule(self->buffer, srule_node->srule);
        }
        break;
    }
}

// Open a buffer with an optional path to load, otherwise empty
static buffer_t* _bview_open_buffer(char* opt_path, int opt_path_len, editor_t* editor) {
    buffer_t* buffer;
    buffer = NULL;
    if (opt_path && opt_path_len > 0) {
        buffer = buffer_new_open(opt_path, opt_path_len);
    }
    if (!buffer) {
        buffer = buffer_new();
    }
    buffer_set_callback(buffer, _bview_buffer_callback, editor);
    if (buffer->tab_width != editor->tab_width) {
        buffer_set_tab_width(buffer, editor->tab_width);
    }
    return buffer;
}

static void _bview_draw_prompt(bview_t* self) {
    _bview_draw_bline(self, self->buffer->first_line, 0);
}

static void _bview_draw_popup(bview_t* self) {
    // TODO
    tb_printf(self->editor->rect_popup, 0, 0, 0, 0, "popup");
}

static void _bview_draw_status(bview_t* self) {
    bview_t* active;
    mark_t* mark;
    active = self->editor->active;
    mark = active->active_cursor->mark;
    // TODO
    tb_printf(self->editor->rect_status, 0, 0, 0, 0,
        "prompt [%s], line %lu/%lu, col %lu/%lu, vcol %lu/%lu, view %dx%d, rect %dx%d",
        self->editor->active == self->editor->prompt ? self->editor->prompt->prompt_label : "",
        mark->bline->line_index,
        active->buffer->line_count,
        mark->col,
        mark->bline->char_count,
        MLE_MARK_COL_TO_VCOL(mark),
        mark->bline->char_vwidth,
        active->viewport_x,
        active->viewport_y,
        active->rect_buffer.w,
        active->rect_buffer.h
    );
}

static void _bview_draw_edit(bview_t* self, int x, int y, int w, int h) {
    int split_w;
    int split_h;
    int min_w;
    int min_h;
    int rect_y;
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
    min_w = self->line_num_width + 3;
    min_h = 2;

    // Ensure renderable
    if (w < min_w || h < min_h
        || x + w > self->editor->w
        || y + h > self->editor->h
    ) {
        return;
    }

    // Render caption
    if (self->buffer->path) {
        tb_printf(self->rect_caption, 0, 0, 0, 0, "%s %c",
            self->buffer->path, self->is_unsaved ? '*' : ' ');
    } else {
        tb_printf(self->rect_caption, 0, 0, 0, 0, "<buffer-%p> %c",
            self->buffer, self->is_unsaved ? '*' : ' ');
    }

    // Render lines and margins
    if (!self->viewport_bline) {
        buffer_get_bline(self->buffer, MLE_MAX(0, self->viewport_y), &self->viewport_bline);
    }
    bline = self->viewport_bline;
    for (rect_y = 0; rect_y < self->rect_buffer.h; rect_y++) {
        if (self->viewport_y + rect_y < 0 || self->viewport_y + rect_y >= self->buffer->line_count) {
            // Draw pre/post blank
            tb_printf(self->rect_lines, 0, rect_y, 0, 0, "%*c", self->line_num_width, '~');
            tb_printf(self->rect_margin_left, 0, rect_y, 0, 0, "%c", ' ');
            tb_printf(self->rect_margin_right, 0, rect_y, 0, 0, "%c", ' ');
            tb_printf(self->rect_buffer, 0, rect_y, 0, 0, "%-*.*s", self->rect_buffer.w, self->rect_buffer.w, " ");
        } else {
            // Draw bline at self->rect_buffer self->viewport_y + rect_y
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
    bint_t i;
    bint_t viewport_x;
    bint_t viewport_x_vcol;

    // Use viewport_x only for current line
    viewport_x = 0;
    viewport_x_vcol = 0;
    if (self->active_cursor->mark->bline == bline) {
        viewport_x = self->viewport_x;
        viewport_x_vcol = self->viewport_x_vcol;
    }

    if (MLE_BVIEW_IS_EDIT(self)) {
        tb_printf(self->rect_lines, 0, rect_y, 0, 0, "%*d", self->line_num_width, (int)(bline->line_index + 1) % (int)pow(10, self->line_num_width));
        tb_printf(self->rect_margin_left, 0, rect_y, 0, 0, "%c", viewport_x > 0 && bline->char_count > 0 ? '^' : ' ');
        tb_printf(self->rect_margin_right, 0, rect_y, 0, 0, "%c", bline->char_vwidth - viewport_x_vcol > self->rect_buffer.w ? '$' : ' ');
    }

    // Render 0 thru rect_buffer.w cell by cell
    for (rect_x = 0, char_col = viewport_x; rect_x < self->rect_buffer.w; rect_x++, char_col++) {
        char_w = 1;
        if (char_col < bline->char_count) {
            tb_utf8_char_to_unicode(&ch, bline->data + bline->char_indexes[char_col]);
            fg = bline->char_styles[char_col].fg;
            bg = bline->char_styles[char_col].bg;
            char_w = char_col == bline->char_count - 1
                ? bline->char_vwidth - bline->char_vcol[char_col]
                : bline->char_vcol[char_col + 1] - bline->char_vcol[char_col];
            if (ch == '\t') {
                ch = ' ';
            } else if (!iswprint(ch)) {
                ch = '?';
            }
        } else {
            tb_utf8_char_to_unicode(&ch, " ");
            fg = 0;
            bg = 0;
            char_w = 1;
        }
        for (i = 0; i < char_w && rect_x < self->rect_buffer.w; i++, rect_x++) {
            tb_change_cell(self->rect_buffer.x + rect_x, self->rect_buffer.y + rect_y, ch, fg, bg);
        }
        rect_x--;
    }
}
