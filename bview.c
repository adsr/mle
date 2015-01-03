#include "mle.h"

static void bview_init(bview_t* self, buffer_t* buffer);
static void bview_deinit(bview_t* self);
static void bview_handle_buffer_action(void* listener, buffer_t* buffer, baction_t* baction);

// Create a new bview
bview_t* bview_new(editor_t* editor, char* opt_path, int opt_path_len) {
    bview_t* self;
    buffer_t* buffer;

    // Open buffer
    if (opt_path && opt_path_len > 0) {
        buffer = buffer_new_open(opt_path, opt_path_len);
    } else {
        buffer = buffer_new();
    }
    if (!buffer) {
        // TODO log error
        return NULL;
    }

    // Allocate and init bview
    self = calloc(1, sizeof(bview_t));
    self->editor = editor;
    self->rect_caption.bg = TB_REVERSE;
    self->rect_lines.fg = TB_YELLOW;
    self->rect_margin_left.fg = TB_RED;
    self->rect_margin_right.fg = TB_RED;
    self->tab_size = editor->tab_size;
    self->tab_to_space = editor->tab_to_space;
    bview_init(self, buffer);

    return self;
}

/** Move and resize a bview to the given position and dimensions */
int bview_resize(bview_t* self, int x, int y, int w, int h) {
    int aw, ah;

    aw = w;
    ah = h;

    if (self->split_child) {
        if (self->split_is_vertical) {
            aw = MLE_MAX(1, (int)((float)aw * self->split_factor));
        } else {
            ah = MLE_MAX(1, (int)((float)ah * self->split_factor));
        }
    }

    self->x = x;
    self->y = y;
    self->w = aw;
    self->h = ah;

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

    return MLE_OK;
}

/** Open a new file in this bview */
int bview_open(bview_t* self, char* path, int path_len) {
    buffer_t* buffer;
    int rc;
    // TODO hook, chance to cancel, save unsaved changes etc
    if ((rc = buffer_open(path, path_len, &buffer)) != MLE_OK) {
        return rc;
    }
    bview_deinit(self);
    buffer_destroy(self->buffer);
    bview_init(self, buffer);
    return MLE_OK;
}

/** Draw bview to screen */
int bview_draw(bview_t* self) {
}

/** Split a bview */
int bview_split(bview_t* self, int is_vertical, float factor, bview_t** optret_bview) {
    bview_t* child;

    if (self->split_child) {
        MLE_RETURN_ERR("bview %p is already split", self);
    }

    // Make child
    child = bview_new(self->editor, NULL, 0);
    bview_init(child, self->buffer);

    // Copy stuff from parent
    child->line_num_width = self->line_num_width;
    child->viewport_x = self->viewport_x;
    child->viewport_y = self->viewport_y;
    //cursor_set_pos(child->active_cursor, self->active_cursor->mark->bline, self->active_cursor->mark->char_offset);

    // Set child on parent
    self->split_child = child;
    self->split_factor = factor;
    self->split_is_vertical = is_vertical;

    // Resize self
    bview_resize(self, self->x, self->y, self->w, self->h);

    if (optret_bview) {
        *optret_bview = child;
    }
    return MLE_OK;
}
