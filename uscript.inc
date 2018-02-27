const char* mle_wren = "class mle {\n\
    foreign static bview_add_cursor(self, bline, col) \n\
    foreign static bview_add_cursor_asleep(self, bline, col) \n\
    foreign static bview_center_viewport_y(self) \n\
    foreign static bview_destroy(self) \n\
    foreign static bview_draw(self) \n\
    foreign static bview_draw_cursor(self, set_real_cursor) \n\
    foreign static bview_get_active_cursor_count(self) \n\
    foreign static bview_get_split_root(self) \n\
    foreign static bview_max_viewport_y(self) \n\
    foreign static bview_new(editor, opt_path, opt_path_len, opt_buffer) \n\
    foreign static bview_open(self, path, path_len) \n\
    foreign static bview_pop_kmap(bview) \n\
    foreign static bview_push_kmap(bview, kmap) \n\
    foreign static bview_rectify_viewport(self) \n\
    foreign static bview_remove_cursor(self, cursor) \n\
    foreign static bview_remove_cursors_except(self, one) \n\
    foreign static bview_resize(self, x, y, w, h) \n\
    foreign static bview_set_syntax(self, opt_syntax) \n\
    foreign static bview_split(self, is_vertical, factor) \n\
    foreign static bview_wake_sleeping_cursors(self) \n\
    foreign static bview_zero_viewport_y(self) \n\
    foreign static cursor_clone(cursor, use_srules) \n\
    foreign static cursor_cut_copy(cursor, is_cut, use_srules, append) \n\
    foreign static cursor_destroy(cursor) \n\
    foreign static cursor_drop_anchor(cursor, use_srules) \n\
    foreign static cursor_get_lo_hi(cursor) \n\
    foreign static cursor_lift_anchor(cursor) \n\
    foreign static cursor_replace(cursor, interactive, opt_regex, opt_replacement) \n\
    foreign static cursor_select_between(cursor, a, b, use_srules) \n\
    foreign static cursor_select_by(cursor, strat) \n\
    foreign static cursor_select_by_bracket(cursor) \n\
    foreign static cursor_select_by_string(cursor) \n\
    foreign static cursor_select_by_word(cursor) \n\
    foreign static cursor_select_by_word_back(cursor) \n\
    foreign static cursor_select_by_word_forward(cursor) \n\
    foreign static cursor_toggle_anchor(cursor, use_srules) \n\
    foreign static cursor_uncut(cursor) \n\
    foreign static editor_bview_edit_count(editor) \n\
    foreign static editor_close_bview(editor, bview) \n\
    foreign static editor_count_bviews_by_buffer(editor, buffer) \n\
    foreign static editor_display(editor) \n\
    foreign static editor_get_input(editor, loop_ctx, ctx) \n\
    foreign static editor_menu(editor, fn_callback, opt_buf_data, opt_buf_data_len, opt_aproc) \n\
    foreign static editor_open_bview(editor, parent, type, opt_path, opt_path_len, make_active, linenum, skip_resize, opt_buffer) \n\
    foreign static editor_prompt(editor, prompt, params) \n\
    foreign static editor_register_cmd(editor, cmd) \n\
    foreign static editor_set_active(editor, bview) \n\
    foreign static mark_clone(self) \n\
    foreign static mark_clone_w_letter(self, letter) \n\
    foreign static mark_delete_after(self, num_chars) \n\
    foreign static mark_delete_before(self, num_chars) \n\
    foreign static mark_delete_between_mark(self, other) \n\
    foreign static mark_destroy(self) \n\
    foreign static mark_find_bracket_pair(self, max_chars) \n\
    foreign static mark_find_bracket_top(self, max_chars) \n\
    foreign static mark_find_next_re(self, re, re_len) \n\
    foreign static mark_find_next_str(self, str, str_len) \n\
    foreign static mark_find_prev_re(self, re, re_len) \n\
    foreign static mark_find_prev_str(self, str, str_len) \n\
    foreign static mark_get_between_mark(self, other) \n\
    foreign static mark_get_char_after(self) \n\
    foreign static mark_get_char_before(self) \n\
    foreign static mark_get_offset(self) \n\
    foreign static mark_insert_after(self, data, data_len) \n\
    foreign static mark_insert_before(self, data, data_len) \n\
    foreign static mark_is_after_col_minus_lefties(self, col) \n\
    foreign static mark_is_at_bol(self) \n\
    foreign static mark_is_at_eol(self) \n\
    foreign static mark_is_at_word_bound(self, side) \n\
    foreign static mark_is_eq(self, other) \n\
    foreign static mark_is_gt(self, other) \n\
    foreign static mark_is_gte(self, other) \n\
    foreign static mark_is_lt(self, other) \n\
    foreign static mark_is_lte(self, other) \n\
    foreign static mark_join(self, other) \n\
    foreign static mark_move_beginning(self) \n\
    foreign static mark_move_bol(self) \n\
    foreign static mark_move_bracket_pair(self, max_chars) \n\
    foreign static mark_move_bracket_pair_ex(self, max_chars) \n\
    foreign static mark_move_bracket_top(self, max_chars) \n\
    foreign static mark_move_bracket_top_ex(self, max_chars) \n\
    foreign static mark_move_by(self, char_delta) \n\
    foreign static mark_move_col(self, col) \n\
    foreign static mark_move_end(self) \n\
    foreign static mark_move_eol(self) \n\
    foreign static mark_move_next_re(self, re, re_len) \n\
    foreign static mark_move_next_re_ex(self, re, re_len) \n\
    foreign static mark_move_next_re_nudge(self, re, re_len) \n\
    foreign static mark_move_next_str(self, str, str_len) \n\
    foreign static mark_move_next_str_ex(self, str, str_len) \n\
    foreign static mark_move_next_str_nudge(self, str, str_len) \n\
    foreign static mark_move_offset(self, offset) \n\
    foreign static mark_move_prev_re(self, re, re_len) \n\
    foreign static mark_move_prev_re_ex(self, re, re_len) \n\
    foreign static mark_move_prev_str(self, str, str_len) \n\
    foreign static mark_move_prev_str_ex(self, str, str_len) \n\
    foreign static mark_move_to(self, line_index, col) \n\
    foreign static mark_move_to_w_bline(self, bline, col) \n\
    foreign static mark_move_vert(self, line_delta) \n\
    foreign static mark_replace(self, num_chars, data, data_len) \n\
    foreign static mark_replace_between_mark(self, other, data, data_len) \n\
    foreign static mark_swap_with_mark(self, other) \n\
} \
";

static void _uscript_bview_add_cursor(WrenVM* vm) {
    int rv;
    bview_t* self;
    bline_t* bline;
    bint_t col;
    cursor_t* optret_cursor = NULL;
    self = (bview_t*)wrenGetSlotPointer(vm, 1);
    bline = (bline_t*)wrenGetSlotPointer(vm, 2);
    col = (bint_t)wrenGetSlotDouble(vm, 3);
    rv = bview_add_cursor(self, bline, col, &optret_cursor);
    wrenEnsureSlots(vm, 3);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotPointer(vm, 2, (void*)optret_cursor);
    wrenInsertInList(vm, 0, -1, 2);
}

static void _uscript_bview_add_cursor_asleep(WrenVM* vm) {
    int rv;
    bview_t* self;
    bline_t* bline;
    bint_t col;
    cursor_t* optret_cursor = NULL;
    self = (bview_t*)wrenGetSlotPointer(vm, 1);
    bline = (bline_t*)wrenGetSlotPointer(vm, 2);
    col = (bint_t)wrenGetSlotDouble(vm, 3);
    rv = bview_add_cursor_asleep(self, bline, col, &optret_cursor);
    wrenEnsureSlots(vm, 3);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotPointer(vm, 2, (void*)optret_cursor);
    wrenInsertInList(vm, 0, -1, 2);
}

static void _uscript_bview_center_viewport_y(WrenVM* vm) {
    int rv;
    bview_t* self;
    self = (bview_t*)wrenGetSlotPointer(vm, 1);
    rv = bview_center_viewport_y(self);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_bview_destroy(WrenVM* vm) {
    int rv;
    bview_t* self;
    self = (bview_t*)wrenGetSlotPointer(vm, 1);
    rv = bview_destroy(self);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_bview_draw(WrenVM* vm) {
    int rv;
    bview_t* self;
    self = (bview_t*)wrenGetSlotPointer(vm, 1);
    rv = bview_draw(self);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_bview_draw_cursor(WrenVM* vm) {
    int rv;
    bview_t* self;
    int set_real_cursor;
    self = (bview_t*)wrenGetSlotPointer(vm, 1);
    set_real_cursor = (int)wrenGetSlotDouble(vm, 2);
    rv = bview_draw_cursor(self, set_real_cursor);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_bview_get_active_cursor_count(WrenVM* vm) {
    int rv;
    bview_t* self;
    self = (bview_t*)wrenGetSlotPointer(vm, 1);
    rv = bview_get_active_cursor_count(self);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_bview_get_split_root(WrenVM* vm) {
    bview_t* rv;
    bview_t* self;
    self = (bview_t*)wrenGetSlotPointer(vm, 1);
    rv = bview_get_split_root(self);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotPointer(vm, 1, (void*)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_bview_max_viewport_y(WrenVM* vm) {
    int rv;
    bview_t* self;
    self = (bview_t*)wrenGetSlotPointer(vm, 1);
    rv = bview_max_viewport_y(self);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_bview_new(WrenVM* vm) {
    bview_t* rv;
    editor_t* editor;
    char* opt_path;
    int opt_path_len;
    buffer_t* opt_buffer;
    editor = (editor_t*)wrenGetSlotPointer(vm, 1);
    opt_path = (char*)wrenGetSlotString(vm, 2);
    opt_path_len = (int)wrenGetSlotDouble(vm, 3);
    opt_buffer = (buffer_t*)wrenGetSlotPointer(vm, 4);
    rv = bview_new(editor, opt_path, opt_path_len, opt_buffer);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotPointer(vm, 1, (void*)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_bview_open(WrenVM* vm) {
    int rv;
    bview_t* self;
    char* path;
    int path_len;
    self = (bview_t*)wrenGetSlotPointer(vm, 1);
    path = (char*)wrenGetSlotString(vm, 2);
    path_len = (int)wrenGetSlotDouble(vm, 3);
    rv = bview_open(self, path, path_len);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

// static void _uscript_bview_pop_kmap(WrenVM* vm) {
// }

// static void _uscript_bview_push_kmap(WrenVM* vm) {
// }

static void _uscript_bview_rectify_viewport(WrenVM* vm) {
    int rv;
    bview_t* self;
    self = (bview_t*)wrenGetSlotPointer(vm, 1);
    rv = bview_rectify_viewport(self);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_bview_remove_cursor(WrenVM* vm) {
    int rv;
    bview_t* self;
    cursor_t* cursor;
    self = (bview_t*)wrenGetSlotPointer(vm, 1);
    cursor = (cursor_t*)wrenGetSlotPointer(vm, 2);
    rv = bview_remove_cursor(self, cursor);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_bview_remove_cursors_except(WrenVM* vm) {
    int rv;
    bview_t* self;
    cursor_t* one;
    self = (bview_t*)wrenGetSlotPointer(vm, 1);
    one = (cursor_t*)wrenGetSlotPointer(vm, 2);
    rv = bview_remove_cursors_except(self, one);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_bview_resize(WrenVM* vm) {
    int rv;
    bview_t* self;
    int x;
    int y;
    int w;
    int h;
    self = (bview_t*)wrenGetSlotPointer(vm, 1);
    x = (int)wrenGetSlotDouble(vm, 2);
    y = (int)wrenGetSlotDouble(vm, 3);
    w = (int)wrenGetSlotDouble(vm, 4);
    h = (int)wrenGetSlotDouble(vm, 5);
    rv = bview_resize(self, x, y, w, h);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_bview_set_syntax(WrenVM* vm) {
    int rv;
    bview_t* self;
    char* opt_syntax;
    self = (bview_t*)wrenGetSlotPointer(vm, 1);
    opt_syntax = (char*)wrenGetSlotString(vm, 2);
    rv = bview_set_syntax(self, opt_syntax);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_bview_split(WrenVM* vm) {
    int rv;
    bview_t* self;
    int is_vertical;
    float factor;
    bview_t* optret_bview = NULL;
    self = (bview_t*)wrenGetSlotPointer(vm, 1);
    is_vertical = (int)wrenGetSlotDouble(vm, 2);
    factor = (float)wrenGetSlotDouble(vm, 3);
    rv = bview_split(self, is_vertical, factor, &optret_bview);
    wrenEnsureSlots(vm, 3);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotPointer(vm, 2, (void*)optret_bview);
    wrenInsertInList(vm, 0, -1, 2);
}

static void _uscript_bview_wake_sleeping_cursors(WrenVM* vm) {
    int rv;
    bview_t* self;
    self = (bview_t*)wrenGetSlotPointer(vm, 1);
    rv = bview_wake_sleeping_cursors(self);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_bview_zero_viewport_y(WrenVM* vm) {
    int rv;
    bview_t* self;
    self = (bview_t*)wrenGetSlotPointer(vm, 1);
    rv = bview_zero_viewport_y(self);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_cursor_clone(WrenVM* vm) {
    int rv;
    cursor_t* cursor;
    int use_srules;
    cursor_t* ret_clone = NULL;
    cursor = (cursor_t*)wrenGetSlotPointer(vm, 1);
    use_srules = (int)wrenGetSlotDouble(vm, 2);
    rv = cursor_clone(cursor, use_srules, &ret_clone);
    wrenEnsureSlots(vm, 3);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotPointer(vm, 2, (void*)ret_clone);
    wrenInsertInList(vm, 0, -1, 2);
}

static void _uscript_cursor_cut_copy(WrenVM* vm) {
    int rv;
    cursor_t* cursor;
    int is_cut;
    int use_srules;
    int append;
    cursor = (cursor_t*)wrenGetSlotPointer(vm, 1);
    is_cut = (int)wrenGetSlotDouble(vm, 2);
    use_srules = (int)wrenGetSlotDouble(vm, 3);
    append = (int)wrenGetSlotDouble(vm, 4);
    rv = cursor_cut_copy(cursor, is_cut, use_srules, append);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_cursor_destroy(WrenVM* vm) {
    int rv;
    cursor_t* cursor;
    cursor = (cursor_t*)wrenGetSlotPointer(vm, 1);
    rv = cursor_destroy(cursor);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_cursor_drop_anchor(WrenVM* vm) {
    int rv;
    cursor_t* cursor;
    int use_srules;
    cursor = (cursor_t*)wrenGetSlotPointer(vm, 1);
    use_srules = (int)wrenGetSlotDouble(vm, 2);
    rv = cursor_drop_anchor(cursor, use_srules);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_cursor_get_lo_hi(WrenVM* vm) {
    int rv;
    cursor_t* cursor;
    mark_t* ret_lo = NULL;
    mark_t* ret_hi = NULL;
    cursor = (cursor_t*)wrenGetSlotPointer(vm, 1);
    rv = cursor_get_lo_hi(cursor, &ret_lo, &ret_hi);
    wrenEnsureSlots(vm, 4);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotPointer(vm, 2, (void*)ret_lo);
    wrenInsertInList(vm, 0, -1, 2);
    wrenSetSlotPointer(vm, 3, (void*)ret_hi);
    wrenInsertInList(vm, 0, -1, 3);
}

static void _uscript_cursor_lift_anchor(WrenVM* vm) {
    int rv;
    cursor_t* cursor;
    cursor = (cursor_t*)wrenGetSlotPointer(vm, 1);
    rv = cursor_lift_anchor(cursor);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_cursor_replace(WrenVM* vm) {
    int rv;
    cursor_t* cursor;
    int interactive;
    char* opt_regex;
    char* opt_replacement;
    cursor = (cursor_t*)wrenGetSlotPointer(vm, 1);
    interactive = (int)wrenGetSlotDouble(vm, 2);
    opt_regex = (char*)wrenGetSlotString(vm, 3);
    opt_replacement = (char*)wrenGetSlotString(vm, 4);
    rv = cursor_replace(cursor, interactive, opt_regex, opt_replacement);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_cursor_select_between(WrenVM* vm) {
    int rv;
    cursor_t* cursor;
    mark_t* a;
    mark_t* b;
    int use_srules;
    cursor = (cursor_t*)wrenGetSlotPointer(vm, 1);
    a = (mark_t*)wrenGetSlotPointer(vm, 2);
    b = (mark_t*)wrenGetSlotPointer(vm, 3);
    use_srules = (int)wrenGetSlotDouble(vm, 4);
    rv = cursor_select_between(cursor, a, b, use_srules);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_cursor_select_by(WrenVM* vm) {
    int rv;
    cursor_t* cursor;
    const char* strat;
    cursor = (cursor_t*)wrenGetSlotPointer(vm, 1);
    strat = (const char*)wrenGetSlotString(vm, 2);
    rv = cursor_select_by(cursor, strat);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_cursor_select_by_bracket(WrenVM* vm) {
    int rv;
    cursor_t* cursor;
    cursor = (cursor_t*)wrenGetSlotPointer(vm, 1);
    rv = cursor_select_by_bracket(cursor);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_cursor_select_by_string(WrenVM* vm) {
    int rv;
    cursor_t* cursor;
    cursor = (cursor_t*)wrenGetSlotPointer(vm, 1);
    rv = cursor_select_by_string(cursor);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_cursor_select_by_word(WrenVM* vm) {
    int rv;
    cursor_t* cursor;
    cursor = (cursor_t*)wrenGetSlotPointer(vm, 1);
    rv = cursor_select_by_word(cursor);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_cursor_select_by_word_back(WrenVM* vm) {
    int rv;
    cursor_t* cursor;
    cursor = (cursor_t*)wrenGetSlotPointer(vm, 1);
    rv = cursor_select_by_word_back(cursor);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_cursor_select_by_word_forward(WrenVM* vm) {
    int rv;
    cursor_t* cursor;
    cursor = (cursor_t*)wrenGetSlotPointer(vm, 1);
    rv = cursor_select_by_word_forward(cursor);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_cursor_toggle_anchor(WrenVM* vm) {
    int rv;
    cursor_t* cursor;
    int use_srules;
    cursor = (cursor_t*)wrenGetSlotPointer(vm, 1);
    use_srules = (int)wrenGetSlotDouble(vm, 2);
    rv = cursor_toggle_anchor(cursor, use_srules);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_cursor_uncut(WrenVM* vm) {
    int rv;
    cursor_t* cursor;
    cursor = (cursor_t*)wrenGetSlotPointer(vm, 1);
    rv = cursor_uncut(cursor);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_editor_bview_edit_count(WrenVM* vm) {
    int rv;
    editor_t* editor;
    editor = (editor_t*)wrenGetSlotPointer(vm, 1);
    rv = editor_bview_edit_count(editor);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_editor_close_bview(WrenVM* vm) {
    int rv;
    editor_t* editor;
    bview_t* bview;
    int optret_num_closed = 0;
    editor = (editor_t*)wrenGetSlotPointer(vm, 1);
    bview = (bview_t*)wrenGetSlotPointer(vm, 2);
    rv = editor_close_bview(editor, bview, &optret_num_closed);
    wrenEnsureSlots(vm, 3);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotDouble(vm, 2, (double)optret_num_closed);
    wrenInsertInList(vm, 0, -1, 2);
}

static void _uscript_editor_count_bviews_by_buffer(WrenVM* vm) {
    int rv;
    editor_t* editor;
    buffer_t* buffer;
    editor = (editor_t*)wrenGetSlotPointer(vm, 1);
    buffer = (buffer_t*)wrenGetSlotPointer(vm, 2);
    rv = editor_count_bviews_by_buffer(editor, buffer);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_editor_display(WrenVM* vm) {
    int rv;
    editor_t* editor;
    editor = (editor_t*)wrenGetSlotPointer(vm, 1);
    rv = editor_display(editor);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

// static void _uscript_editor_get_input(WrenVM* vm) {
// }

// static void _uscript_editor_menu(WrenVM* vm) {
// }

static void _uscript_editor_open_bview(WrenVM* vm) {
    int rv;
    editor_t* editor;
    bview_t* parent;
    int type;
    char* opt_path;
    int opt_path_len;
    int make_active;
    bint_t linenum;
    bview_rect_t* opt_rect;
    buffer_t* opt_buffer;
    bview_t* optret_bview = NULL;
    editor = (editor_t*)wrenGetSlotPointer(vm, 1);
    parent = (bview_t*)wrenGetSlotPointer(vm, 2);
    type = (int)wrenGetSlotDouble(vm, 3);
    opt_path = (char*)wrenGetSlotString(vm, 4);
    opt_path_len = (int)wrenGetSlotDouble(vm, 5);
    make_active = (int)wrenGetSlotDouble(vm, 6);
    linenum = (bint_t)wrenGetSlotDouble(vm, 7);
    opt_rect = (bview_rect_t*)wrenGetSlotPointer(vm, 8);
    opt_buffer = (buffer_t*)wrenGetSlotPointer(vm, 9);
    rv = editor_open_bview(editor, parent, type, opt_path, opt_path_len, make_active, linenum, opt_rect, opt_buffer, &optret_bview);
    wrenEnsureSlots(vm, 3);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotPointer(vm, 2, (void*)optret_bview);
    wrenInsertInList(vm, 0, -1, 2);
}

// static void _uscript_editor_prompt(WrenVM* vm) {
// }

// static void _uscript_editor_register_cmd(WrenVM* vm) {
// }

static void _uscript_editor_set_active(WrenVM* vm) {
    int rv;
    editor_t* editor;
    bview_t* bview;
    editor = (editor_t*)wrenGetSlotPointer(vm, 1);
    bview = (bview_t*)wrenGetSlotPointer(vm, 2);
    rv = editor_set_active(editor, bview);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_clone(WrenVM* vm) {
    int rv;
    mark_t* self;
    mark_t* ret_mark = NULL;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    rv = mark_clone(self, &ret_mark);
    wrenEnsureSlots(vm, 3);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotPointer(vm, 2, (void*)ret_mark);
    wrenInsertInList(vm, 0, -1, 2);
}

static void _uscript_mark_clone_w_letter(WrenVM* vm) {
    int rv;
    mark_t* self;
    char letter;
    mark_t* ret_mark = NULL;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    letter = (char)wrenGetSlotDouble(vm, 2);
    rv = mark_clone_w_letter(self, letter, &ret_mark);
    wrenEnsureSlots(vm, 3);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotPointer(vm, 2, (void*)ret_mark);
    wrenInsertInList(vm, 0, -1, 2);
}

static void _uscript_mark_delete_after(WrenVM* vm) {
    int rv;
    mark_t* self;
    bint_t num_chars;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    num_chars = (bint_t)wrenGetSlotDouble(vm, 2);
    rv = mark_delete_after(self, num_chars);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_delete_before(WrenVM* vm) {
    int rv;
    mark_t* self;
    bint_t num_chars;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    num_chars = (bint_t)wrenGetSlotDouble(vm, 2);
    rv = mark_delete_before(self, num_chars);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_delete_between_mark(WrenVM* vm) {
    int rv;
    mark_t* self;
    mark_t* other;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    other = (mark_t*)wrenGetSlotPointer(vm, 2);
    rv = mark_delete_between_mark(self, other);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_destroy(WrenVM* vm) {
    int rv;
    mark_t* self;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    rv = mark_destroy(self);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_find_bracket_pair(WrenVM* vm) {
    int rv;
    mark_t* self;
    bint_t max_chars;
    bline_t* ret_line = NULL;
    bint_t ret_col = 0;
    bint_t ret_brkt = 0;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    max_chars = (bint_t)wrenGetSlotDouble(vm, 2);
    rv = mark_find_bracket_pair(self, max_chars, &ret_line, &ret_col, &ret_brkt);
    wrenEnsureSlots(vm, 5);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotPointer(vm, 2, (void*)ret_line);
    wrenInsertInList(vm, 0, -1, 2);
    wrenSetSlotDouble(vm, 3, (double)ret_col);
    wrenInsertInList(vm, 0, -1, 3);
    wrenSetSlotDouble(vm, 4, (double)ret_brkt);
    wrenInsertInList(vm, 0, -1, 4);
}

static void _uscript_mark_find_bracket_top(WrenVM* vm) {
    int rv;
    mark_t* self;
    bint_t max_chars;
    bline_t* ret_line = NULL;
    bint_t ret_col = 0;
    bint_t ret_brkt = 0;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    max_chars = (bint_t)wrenGetSlotDouble(vm, 2);
    rv = mark_find_bracket_top(self, max_chars, &ret_line, &ret_col, &ret_brkt);
    wrenEnsureSlots(vm, 5);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotPointer(vm, 2, (void*)ret_line);
    wrenInsertInList(vm, 0, -1, 2);
    wrenSetSlotDouble(vm, 3, (double)ret_col);
    wrenInsertInList(vm, 0, -1, 3);
    wrenSetSlotDouble(vm, 4, (double)ret_brkt);
    wrenInsertInList(vm, 0, -1, 4);
}

static void _uscript_mark_find_next_re(WrenVM* vm) {
    int rv;
    mark_t* self;
    char* re;
    bint_t re_len;
    bline_t* ret_line = NULL;
    bint_t ret_col = 0;
    bint_t ret_num_chars = 0;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    re = (char*)wrenGetSlotString(vm, 2);
    re_len = (bint_t)wrenGetSlotDouble(vm, 3);
    rv = mark_find_next_re(self, re, re_len, &ret_line, &ret_col, &ret_num_chars);
    wrenEnsureSlots(vm, 5);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotPointer(vm, 2, (void*)ret_line);
    wrenInsertInList(vm, 0, -1, 2);
    wrenSetSlotDouble(vm, 3, (double)ret_col);
    wrenInsertInList(vm, 0, -1, 3);
    wrenSetSlotDouble(vm, 4, (double)ret_num_chars);
    wrenInsertInList(vm, 0, -1, 4);
}

static void _uscript_mark_find_next_str(WrenVM* vm) {
    int rv;
    mark_t* self;
    char* str;
    bint_t str_len;
    bline_t* ret_line = NULL;
    bint_t ret_col = 0;
    bint_t ret_num_chars = 0;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    str = (char*)wrenGetSlotString(vm, 2);
    str_len = (bint_t)wrenGetSlotDouble(vm, 3);
    rv = mark_find_next_str(self, str, str_len, &ret_line, &ret_col, &ret_num_chars);
    wrenEnsureSlots(vm, 5);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotPointer(vm, 2, (void*)ret_line);
    wrenInsertInList(vm, 0, -1, 2);
    wrenSetSlotDouble(vm, 3, (double)ret_col);
    wrenInsertInList(vm, 0, -1, 3);
    wrenSetSlotDouble(vm, 4, (double)ret_num_chars);
    wrenInsertInList(vm, 0, -1, 4);
}

static void _uscript_mark_find_prev_re(WrenVM* vm) {
    int rv;
    mark_t* self;
    char* re;
    bint_t re_len;
    bline_t* ret_line = NULL;
    bint_t ret_col = 0;
    bint_t ret_num_chars = 0;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    re = (char*)wrenGetSlotString(vm, 2);
    re_len = (bint_t)wrenGetSlotDouble(vm, 3);
    rv = mark_find_prev_re(self, re, re_len, &ret_line, &ret_col, &ret_num_chars);
    wrenEnsureSlots(vm, 5);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotPointer(vm, 2, (void*)ret_line);
    wrenInsertInList(vm, 0, -1, 2);
    wrenSetSlotDouble(vm, 3, (double)ret_col);
    wrenInsertInList(vm, 0, -1, 3);
    wrenSetSlotDouble(vm, 4, (double)ret_num_chars);
    wrenInsertInList(vm, 0, -1, 4);
}

static void _uscript_mark_find_prev_str(WrenVM* vm) {
    int rv;
    mark_t* self;
    char* str;
    bint_t str_len;
    bline_t* ret_line = NULL;
    bint_t ret_col = 0;
    bint_t ret_num_chars = 0;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    str = (char*)wrenGetSlotString(vm, 2);
    str_len = (bint_t)wrenGetSlotDouble(vm, 3);
    rv = mark_find_prev_str(self, str, str_len, &ret_line, &ret_col, &ret_num_chars);
    wrenEnsureSlots(vm, 5);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotPointer(vm, 2, (void*)ret_line);
    wrenInsertInList(vm, 0, -1, 2);
    wrenSetSlotDouble(vm, 3, (double)ret_col);
    wrenInsertInList(vm, 0, -1, 3);
    wrenSetSlotDouble(vm, 4, (double)ret_num_chars);
    wrenInsertInList(vm, 0, -1, 4);
}

static void _uscript_mark_get_between_mark(WrenVM* vm) {
    int rv;
    mark_t* self;
    mark_t* other;
    char* ret_str = NULL;
    bint_t ret_str_len = 0;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    other = (mark_t*)wrenGetSlotPointer(vm, 2);
    rv = mark_get_between_mark(self, other, &ret_str, &ret_str_len);
    wrenEnsureSlots(vm, 4);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotPointer(vm, 2, (void*)ret_str);
    wrenInsertInList(vm, 0, -1, 2);
    wrenSetSlotDouble(vm, 3, (double)ret_str_len);
    wrenInsertInList(vm, 0, -1, 3);
}

static void _uscript_mark_get_char_after(WrenVM* vm) {
    int rv;
    mark_t* self;
    uint32_t ret_char = 0;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    rv = mark_get_char_after(self, &ret_char);
    wrenEnsureSlots(vm, 3);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotDouble(vm, 2, (double)ret_char);
    wrenInsertInList(vm, 0, -1, 2);
}

static void _uscript_mark_get_char_before(WrenVM* vm) {
    int rv;
    mark_t* self;
    uint32_t ret_char = 0;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    rv = mark_get_char_before(self, &ret_char);
    wrenEnsureSlots(vm, 3);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotDouble(vm, 2, (double)ret_char);
    wrenInsertInList(vm, 0, -1, 2);
}

static void _uscript_mark_get_offset(WrenVM* vm) {
    int rv;
    mark_t* self;
    bint_t ret_offset = 0;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    rv = mark_get_offset(self, &ret_offset);
    wrenEnsureSlots(vm, 3);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotDouble(vm, 2, (double)ret_offset);
    wrenInsertInList(vm, 0, -1, 2);
}

static void _uscript_mark_insert_after(WrenVM* vm) {
    int rv;
    mark_t* self;
    char* data;
    bint_t data_len;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    data = (char*)wrenGetSlotString(vm, 2);
    data_len = (bint_t)wrenGetSlotDouble(vm, 3);
    rv = mark_insert_after(self, data, data_len);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_insert_before(WrenVM* vm) {
    int rv;
    mark_t* self;
    char* data;
    bint_t data_len;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    data = (char*)wrenGetSlotString(vm, 2);
    data_len = (bint_t)wrenGetSlotDouble(vm, 3);
    rv = mark_insert_before(self, data, data_len);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_is_after_col_minus_lefties(WrenVM* vm) {
    int rv;
    mark_t* self;
    bint_t col;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    col = (bint_t)wrenGetSlotDouble(vm, 2);
    rv = mark_is_after_col_minus_lefties(self, col);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_is_at_bol(WrenVM* vm) {
    int rv;
    mark_t* self;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    rv = mark_is_at_bol(self);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_is_at_eol(WrenVM* vm) {
    int rv;
    mark_t* self;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    rv = mark_is_at_eol(self);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_is_at_word_bound(WrenVM* vm) {
    int rv;
    mark_t* self;
    int side;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    side = (int)wrenGetSlotDouble(vm, 2);
    rv = mark_is_at_word_bound(self, side);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_is_eq(WrenVM* vm) {
    int rv;
    mark_t* self;
    mark_t* other;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    other = (mark_t*)wrenGetSlotPointer(vm, 2);
    rv = mark_is_eq(self, other);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_is_gt(WrenVM* vm) {
    int rv;
    mark_t* self;
    mark_t* other;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    other = (mark_t*)wrenGetSlotPointer(vm, 2);
    rv = mark_is_gt(self, other);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_is_gte(WrenVM* vm) {
    int rv;
    mark_t* self;
    mark_t* other;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    other = (mark_t*)wrenGetSlotPointer(vm, 2);
    rv = mark_is_gte(self, other);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_is_lt(WrenVM* vm) {
    int rv;
    mark_t* self;
    mark_t* other;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    other = (mark_t*)wrenGetSlotPointer(vm, 2);
    rv = mark_is_lt(self, other);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_is_lte(WrenVM* vm) {
    int rv;
    mark_t* self;
    mark_t* other;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    other = (mark_t*)wrenGetSlotPointer(vm, 2);
    rv = mark_is_lte(self, other);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_join(WrenVM* vm) {
    int rv;
    mark_t* self;
    mark_t* other;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    other = (mark_t*)wrenGetSlotPointer(vm, 2);
    rv = mark_join(self, other);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_move_beginning(WrenVM* vm) {
    int rv;
    mark_t* self;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    rv = mark_move_beginning(self);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_move_bol(WrenVM* vm) {
    int rv;
    mark_t* self;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    rv = mark_move_bol(self);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_move_bracket_pair(WrenVM* vm) {
    int rv;
    mark_t* self;
    bint_t max_chars;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    max_chars = (bint_t)wrenGetSlotDouble(vm, 2);
    rv = mark_move_bracket_pair(self, max_chars);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_move_bracket_pair_ex(WrenVM* vm) {
    int rv;
    mark_t* self;
    bint_t max_chars;
    bline_t* optret_line = NULL;
    bint_t optret_col = 0;
    bint_t optret_num_chars = 0;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    max_chars = (bint_t)wrenGetSlotDouble(vm, 2);
    rv = mark_move_bracket_pair_ex(self, max_chars, &optret_line, &optret_col, &optret_num_chars);
    wrenEnsureSlots(vm, 5);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotPointer(vm, 2, (void*)optret_line);
    wrenInsertInList(vm, 0, -1, 2);
    wrenSetSlotDouble(vm, 3, (double)optret_col);
    wrenInsertInList(vm, 0, -1, 3);
    wrenSetSlotDouble(vm, 4, (double)optret_num_chars);
    wrenInsertInList(vm, 0, -1, 4);
}

static void _uscript_mark_move_bracket_top(WrenVM* vm) {
    int rv;
    mark_t* self;
    bint_t max_chars;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    max_chars = (bint_t)wrenGetSlotDouble(vm, 2);
    rv = mark_move_bracket_top(self, max_chars);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_move_bracket_top_ex(WrenVM* vm) {
    int rv;
    mark_t* self;
    bint_t max_chars;
    bline_t* optret_line = NULL;
    bint_t optret_col = 0;
    bint_t optret_num_chars = 0;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    max_chars = (bint_t)wrenGetSlotDouble(vm, 2);
    rv = mark_move_bracket_top_ex(self, max_chars, &optret_line, &optret_col, &optret_num_chars);
    wrenEnsureSlots(vm, 5);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotPointer(vm, 2, (void*)optret_line);
    wrenInsertInList(vm, 0, -1, 2);
    wrenSetSlotDouble(vm, 3, (double)optret_col);
    wrenInsertInList(vm, 0, -1, 3);
    wrenSetSlotDouble(vm, 4, (double)optret_num_chars);
    wrenInsertInList(vm, 0, -1, 4);
}

static void _uscript_mark_move_by(WrenVM* vm) {
    int rv;
    mark_t* self;
    bint_t char_delta;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    char_delta = (bint_t)wrenGetSlotDouble(vm, 2);
    rv = mark_move_by(self, char_delta);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_move_col(WrenVM* vm) {
    int rv;
    mark_t* self;
    bint_t col;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    col = (bint_t)wrenGetSlotDouble(vm, 2);
    rv = mark_move_col(self, col);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_move_end(WrenVM* vm) {
    int rv;
    mark_t* self;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    rv = mark_move_end(self);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_move_eol(WrenVM* vm) {
    int rv;
    mark_t* self;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    rv = mark_move_eol(self);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_move_next_re(WrenVM* vm) {
    int rv;
    mark_t* self;
    char* re;
    bint_t re_len;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    re = (char*)wrenGetSlotString(vm, 2);
    re_len = (bint_t)wrenGetSlotDouble(vm, 3);
    rv = mark_move_next_re(self, re, re_len);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_move_next_re_ex(WrenVM* vm) {
    int rv;
    mark_t* self;
    char* re;
    bint_t re_len;
    bline_t* optret_line = NULL;
    bint_t optret_col = 0;
    bint_t optret_num_chars = 0;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    re = (char*)wrenGetSlotString(vm, 2);
    re_len = (bint_t)wrenGetSlotDouble(vm, 3);
    rv = mark_move_next_re_ex(self, re, re_len, &optret_line, &optret_col, &optret_num_chars);
    wrenEnsureSlots(vm, 5);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotPointer(vm, 2, (void*)optret_line);
    wrenInsertInList(vm, 0, -1, 2);
    wrenSetSlotDouble(vm, 3, (double)optret_col);
    wrenInsertInList(vm, 0, -1, 3);
    wrenSetSlotDouble(vm, 4, (double)optret_num_chars);
    wrenInsertInList(vm, 0, -1, 4);
}

static void _uscript_mark_move_next_re_nudge(WrenVM* vm) {
    int rv;
    mark_t* self;
    char* re;
    bint_t re_len;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    re = (char*)wrenGetSlotString(vm, 2);
    re_len = (bint_t)wrenGetSlotDouble(vm, 3);
    rv = mark_move_next_re_nudge(self, re, re_len);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_move_next_str(WrenVM* vm) {
    int rv;
    mark_t* self;
    char* str;
    bint_t str_len;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    str = (char*)wrenGetSlotString(vm, 2);
    str_len = (bint_t)wrenGetSlotDouble(vm, 3);
    rv = mark_move_next_str(self, str, str_len);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_move_next_str_ex(WrenVM* vm) {
    int rv;
    mark_t* self;
    char* str;
    bint_t str_len;
    bline_t* optret_line = NULL;
    bint_t optret_col = 0;
    bint_t optret_num_chars = 0;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    str = (char*)wrenGetSlotString(vm, 2);
    str_len = (bint_t)wrenGetSlotDouble(vm, 3);
    rv = mark_move_next_str_ex(self, str, str_len, &optret_line, &optret_col, &optret_num_chars);
    wrenEnsureSlots(vm, 5);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotPointer(vm, 2, (void*)optret_line);
    wrenInsertInList(vm, 0, -1, 2);
    wrenSetSlotDouble(vm, 3, (double)optret_col);
    wrenInsertInList(vm, 0, -1, 3);
    wrenSetSlotDouble(vm, 4, (double)optret_num_chars);
    wrenInsertInList(vm, 0, -1, 4);
}

static void _uscript_mark_move_next_str_nudge(WrenVM* vm) {
    int rv;
    mark_t* self;
    char* str;
    bint_t str_len;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    str = (char*)wrenGetSlotString(vm, 2);
    str_len = (bint_t)wrenGetSlotDouble(vm, 3);
    rv = mark_move_next_str_nudge(self, str, str_len);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_move_offset(WrenVM* vm) {
    int rv;
    mark_t* self;
    bint_t offset;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    offset = (bint_t)wrenGetSlotDouble(vm, 2);
    rv = mark_move_offset(self, offset);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_move_prev_re(WrenVM* vm) {
    int rv;
    mark_t* self;
    char* re;
    bint_t re_len;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    re = (char*)wrenGetSlotString(vm, 2);
    re_len = (bint_t)wrenGetSlotDouble(vm, 3);
    rv = mark_move_prev_re(self, re, re_len);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_move_prev_re_ex(WrenVM* vm) {
    int rv;
    mark_t* self;
    char* re;
    bint_t re_len;
    bline_t* optret_line = NULL;
    bint_t optret_col = 0;
    bint_t optret_num_chars = 0;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    re = (char*)wrenGetSlotString(vm, 2);
    re_len = (bint_t)wrenGetSlotDouble(vm, 3);
    rv = mark_move_prev_re_ex(self, re, re_len, &optret_line, &optret_col, &optret_num_chars);
    wrenEnsureSlots(vm, 5);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotPointer(vm, 2, (void*)optret_line);
    wrenInsertInList(vm, 0, -1, 2);
    wrenSetSlotDouble(vm, 3, (double)optret_col);
    wrenInsertInList(vm, 0, -1, 3);
    wrenSetSlotDouble(vm, 4, (double)optret_num_chars);
    wrenInsertInList(vm, 0, -1, 4);
}

static void _uscript_mark_move_prev_str(WrenVM* vm) {
    int rv;
    mark_t* self;
    char* str;
    bint_t str_len;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    str = (char*)wrenGetSlotString(vm, 2);
    str_len = (bint_t)wrenGetSlotDouble(vm, 3);
    rv = mark_move_prev_str(self, str, str_len);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_move_prev_str_ex(WrenVM* vm) {
    int rv;
    mark_t* self;
    char* str;
    bint_t str_len;
    bline_t* optret_line = NULL;
    bint_t optret_col = 0;
    bint_t optret_num_chars = 0;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    str = (char*)wrenGetSlotString(vm, 2);
    str_len = (bint_t)wrenGetSlotDouble(vm, 3);
    rv = mark_move_prev_str_ex(self, str, str_len, &optret_line, &optret_col, &optret_num_chars);
    wrenEnsureSlots(vm, 5);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotPointer(vm, 2, (void*)optret_line);
    wrenInsertInList(vm, 0, -1, 2);
    wrenSetSlotDouble(vm, 3, (double)optret_col);
    wrenInsertInList(vm, 0, -1, 3);
    wrenSetSlotDouble(vm, 4, (double)optret_num_chars);
    wrenInsertInList(vm, 0, -1, 4);
}

static void _uscript_mark_move_to(WrenVM* vm) {
    int rv;
    mark_t* self;
    bint_t line_index;
    bint_t col;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    line_index = (bint_t)wrenGetSlotDouble(vm, 2);
    col = (bint_t)wrenGetSlotDouble(vm, 3);
    rv = mark_move_to(self, line_index, col);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_move_to_w_bline(WrenVM* vm) {
    int rv;
    mark_t* self;
    bline_t* bline;
    bint_t col;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    bline = (bline_t*)wrenGetSlotPointer(vm, 2);
    col = (bint_t)wrenGetSlotDouble(vm, 3);
    rv = mark_move_to_w_bline(self, bline, col);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_move_vert(WrenVM* vm) {
    int rv;
    mark_t* self;
    bint_t line_delta;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    line_delta = (bint_t)wrenGetSlotDouble(vm, 2);
    rv = mark_move_vert(self, line_delta);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_replace(WrenVM* vm) {
    int rv;
    mark_t* self;
    bint_t num_chars;
    char* data;
    bint_t data_len;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    num_chars = (bint_t)wrenGetSlotDouble(vm, 2);
    data = (char*)wrenGetSlotString(vm, 3);
    data_len = (bint_t)wrenGetSlotDouble(vm, 4);
    rv = mark_replace(self, num_chars, data, data_len);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_replace_between_mark(WrenVM* vm) {
    int rv;
    mark_t* self;
    mark_t* other;
    char* data;
    bint_t data_len;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    other = (mark_t*)wrenGetSlotPointer(vm, 2);
    data = (char*)wrenGetSlotString(vm, 3);
    data_len = (bint_t)wrenGetSlotDouble(vm, 4);
    rv = mark_replace_between_mark(self, other, data, data_len);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static void _uscript_mark_swap_with_mark(WrenVM* vm) {
    int rv;
    mark_t* self;
    mark_t* other;
    self = (mark_t*)wrenGetSlotPointer(vm, 1);
    other = (mark_t*)wrenGetSlotPointer(vm, 2);
    rv = mark_swap_with_mark(self, other);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
}

static WrenForeignMethodFn _uscript_bind_method(WrenVM* vm, const char* module, const char* class, bool is_static, const char* sig) {
    if (!is_static
        || strcmp(module, "main") != 0
        || strcmp(class, "mle") != 0
    ) {
        return NULL;
    }
    if (0) {
    } else if (strncmp(sig, "bview_add_cursor(", 17) == 0) {
        return _uscript_bview_add_cursor;
    } else if (strncmp(sig, "bview_add_cursor_asleep(", 24) == 0) {
        return _uscript_bview_add_cursor_asleep;
    } else if (strncmp(sig, "bview_center_viewport_y(", 24) == 0) {
        return _uscript_bview_center_viewport_y;
    } else if (strncmp(sig, "bview_destroy(", 14) == 0) {
        return _uscript_bview_destroy;
    } else if (strncmp(sig, "bview_draw(", 11) == 0) {
        return _uscript_bview_draw;
    } else if (strncmp(sig, "bview_draw_cursor(", 18) == 0) {
        return _uscript_bview_draw_cursor;
    } else if (strncmp(sig, "bview_get_active_cursor_count(", 30) == 0) {
        return _uscript_bview_get_active_cursor_count;
    } else if (strncmp(sig, "bview_get_split_root(", 21) == 0) {
        return _uscript_bview_get_split_root;
    } else if (strncmp(sig, "bview_max_viewport_y(", 21) == 0) {
        return _uscript_bview_max_viewport_y;
    } else if (strncmp(sig, "bview_new(", 10) == 0) {
        return _uscript_bview_new;
    } else if (strncmp(sig, "bview_open(", 11) == 0) {
        return _uscript_bview_open;
    } else if (strncmp(sig, "bview_pop_kmap(", 15) == 0) {
        return _uscript_bview_pop_kmap;
    } else if (strncmp(sig, "bview_push_kmap(", 16) == 0) {
        return _uscript_bview_push_kmap;
    } else if (strncmp(sig, "bview_rectify_viewport(", 23) == 0) {
        return _uscript_bview_rectify_viewport;
    } else if (strncmp(sig, "bview_remove_cursor(", 20) == 0) {
        return _uscript_bview_remove_cursor;
    } else if (strncmp(sig, "bview_remove_cursors_except(", 28) == 0) {
        return _uscript_bview_remove_cursors_except;
    } else if (strncmp(sig, "bview_resize(", 13) == 0) {
        return _uscript_bview_resize;
    } else if (strncmp(sig, "bview_set_syntax(", 17) == 0) {
        return _uscript_bview_set_syntax;
    } else if (strncmp(sig, "bview_split(", 12) == 0) {
        return _uscript_bview_split;
    } else if (strncmp(sig, "bview_wake_sleeping_cursors(", 28) == 0) {
        return _uscript_bview_wake_sleeping_cursors;
    } else if (strncmp(sig, "bview_zero_viewport_y(", 22) == 0) {
        return _uscript_bview_zero_viewport_y;
    } else if (strncmp(sig, "cursor_clone(", 13) == 0) {
        return _uscript_cursor_clone;
    } else if (strncmp(sig, "cursor_cut_copy(", 16) == 0) {
        return _uscript_cursor_cut_copy;
    } else if (strncmp(sig, "cursor_destroy(", 15) == 0) {
        return _uscript_cursor_destroy;
    } else if (strncmp(sig, "cursor_drop_anchor(", 19) == 0) {
        return _uscript_cursor_drop_anchor;
    } else if (strncmp(sig, "cursor_get_lo_hi(", 17) == 0) {
        return _uscript_cursor_get_lo_hi;
    } else if (strncmp(sig, "cursor_lift_anchor(", 19) == 0) {
        return _uscript_cursor_lift_anchor;
    } else if (strncmp(sig, "cursor_replace(", 15) == 0) {
        return _uscript_cursor_replace;
    } else if (strncmp(sig, "cursor_select_between(", 22) == 0) {
        return _uscript_cursor_select_between;
    } else if (strncmp(sig, "cursor_select_by(", 17) == 0) {
        return _uscript_cursor_select_by;
    } else if (strncmp(sig, "cursor_select_by_bracket(", 25) == 0) {
        return _uscript_cursor_select_by_bracket;
    } else if (strncmp(sig, "cursor_select_by_string(", 24) == 0) {
        return _uscript_cursor_select_by_string;
    } else if (strncmp(sig, "cursor_select_by_word(", 22) == 0) {
        return _uscript_cursor_select_by_word;
    } else if (strncmp(sig, "cursor_select_by_word_back(", 27) == 0) {
        return _uscript_cursor_select_by_word_back;
    } else if (strncmp(sig, "cursor_select_by_word_forward(", 30) == 0) {
        return _uscript_cursor_select_by_word_forward;
    } else if (strncmp(sig, "cursor_toggle_anchor(", 21) == 0) {
        return _uscript_cursor_toggle_anchor;
    } else if (strncmp(sig, "cursor_uncut(", 13) == 0) {
        return _uscript_cursor_uncut;
    } else if (strncmp(sig, "editor_bview_edit_count(", 24) == 0) {
        return _uscript_editor_bview_edit_count;
    } else if (strncmp(sig, "editor_close_bview(", 19) == 0) {
        return _uscript_editor_close_bview;
    } else if (strncmp(sig, "editor_count_bviews_by_buffer(", 30) == 0) {
        return _uscript_editor_count_bviews_by_buffer;
    } else if (strncmp(sig, "editor_display(", 15) == 0) {
        return _uscript_editor_display;
    } else if (strncmp(sig, "editor_get_input(", 17) == 0) {
        return _uscript_editor_get_input;
    } else if (strncmp(sig, "editor_menu(", 12) == 0) {
        return _uscript_editor_menu;
    } else if (strncmp(sig, "editor_open_bview(", 18) == 0) {
        return _uscript_editor_open_bview;
    } else if (strncmp(sig, "editor_prompt(", 14) == 0) {
        return _uscript_editor_prompt;
    } else if (strncmp(sig, "editor_register_cmd(", 20) == 0) {
        return _uscript_editor_register_cmd;
    } else if (strncmp(sig, "editor_set_active(", 18) == 0) {
        return _uscript_editor_set_active;
    } else if (strncmp(sig, "mark_clone(", 11) == 0) {
        return _uscript_mark_clone;
    } else if (strncmp(sig, "mark_clone_w_letter(", 20) == 0) {
        return _uscript_mark_clone_w_letter;
    } else if (strncmp(sig, "mark_delete_after(", 18) == 0) {
        return _uscript_mark_delete_after;
    } else if (strncmp(sig, "mark_delete_before(", 19) == 0) {
        return _uscript_mark_delete_before;
    } else if (strncmp(sig, "mark_delete_between_mark(", 25) == 0) {
        return _uscript_mark_delete_between_mark;
    } else if (strncmp(sig, "mark_destroy(", 13) == 0) {
        return _uscript_mark_destroy;
    } else if (strncmp(sig, "mark_find_bracket_pair(", 23) == 0) {
        return _uscript_mark_find_bracket_pair;
    } else if (strncmp(sig, "mark_find_bracket_top(", 22) == 0) {
        return _uscript_mark_find_bracket_top;
    } else if (strncmp(sig, "mark_find_next_re(", 18) == 0) {
        return _uscript_mark_find_next_re;
    } else if (strncmp(sig, "mark_find_next_str(", 19) == 0) {
        return _uscript_mark_find_next_str;
    } else if (strncmp(sig, "mark_find_prev_re(", 18) == 0) {
        return _uscript_mark_find_prev_re;
    } else if (strncmp(sig, "mark_find_prev_str(", 19) == 0) {
        return _uscript_mark_find_prev_str;
    } else if (strncmp(sig, "mark_get_between_mark(", 22) == 0) {
        return _uscript_mark_get_between_mark;
    } else if (strncmp(sig, "mark_get_char_after(", 20) == 0) {
        return _uscript_mark_get_char_after;
    } else if (strncmp(sig, "mark_get_char_before(", 21) == 0) {
        return _uscript_mark_get_char_before;
    } else if (strncmp(sig, "mark_get_offset(", 16) == 0) {
        return _uscript_mark_get_offset;
    } else if (strncmp(sig, "mark_insert_after(", 18) == 0) {
        return _uscript_mark_insert_after;
    } else if (strncmp(sig, "mark_insert_before(", 19) == 0) {
        return _uscript_mark_insert_before;
    } else if (strncmp(sig, "mark_is_after_col_minus_lefties(", 32) == 0) {
        return _uscript_mark_is_after_col_minus_lefties;
    } else if (strncmp(sig, "mark_is_at_bol(", 15) == 0) {
        return _uscript_mark_is_at_bol;
    } else if (strncmp(sig, "mark_is_at_eol(", 15) == 0) {
        return _uscript_mark_is_at_eol;
    } else if (strncmp(sig, "mark_is_at_word_bound(", 22) == 0) {
        return _uscript_mark_is_at_word_bound;
    } else if (strncmp(sig, "mark_is_eq(", 11) == 0) {
        return _uscript_mark_is_eq;
    } else if (strncmp(sig, "mark_is_gt(", 11) == 0) {
        return _uscript_mark_is_gt;
    } else if (strncmp(sig, "mark_is_gte(", 12) == 0) {
        return _uscript_mark_is_gte;
    } else if (strncmp(sig, "mark_is_lt(", 11) == 0) {
        return _uscript_mark_is_lt;
    } else if (strncmp(sig, "mark_is_lte(", 12) == 0) {
        return _uscript_mark_is_lte;
    } else if (strncmp(sig, "mark_join(", 10) == 0) {
        return _uscript_mark_join;
    } else if (strncmp(sig, "mark_move_beginning(", 20) == 0) {
        return _uscript_mark_move_beginning;
    } else if (strncmp(sig, "mark_move_bol(", 14) == 0) {
        return _uscript_mark_move_bol;
    } else if (strncmp(sig, "mark_move_bracket_pair(", 23) == 0) {
        return _uscript_mark_move_bracket_pair;
    } else if (strncmp(sig, "mark_move_bracket_pair_ex(", 26) == 0) {
        return _uscript_mark_move_bracket_pair_ex;
    } else if (strncmp(sig, "mark_move_bracket_top(", 22) == 0) {
        return _uscript_mark_move_bracket_top;
    } else if (strncmp(sig, "mark_move_bracket_top_ex(", 25) == 0) {
        return _uscript_mark_move_bracket_top_ex;
    } else if (strncmp(sig, "mark_move_by(", 13) == 0) {
        return _uscript_mark_move_by;
    } else if (strncmp(sig, "mark_move_col(", 14) == 0) {
        return _uscript_mark_move_col;
    } else if (strncmp(sig, "mark_move_end(", 14) == 0) {
        return _uscript_mark_move_end;
    } else if (strncmp(sig, "mark_move_eol(", 14) == 0) {
        return _uscript_mark_move_eol;
    } else if (strncmp(sig, "mark_move_next_re(", 18) == 0) {
        return _uscript_mark_move_next_re;
    } else if (strncmp(sig, "mark_move_next_re_ex(", 21) == 0) {
        return _uscript_mark_move_next_re_ex;
    } else if (strncmp(sig, "mark_move_next_re_nudge(", 24) == 0) {
        return _uscript_mark_move_next_re_nudge;
    } else if (strncmp(sig, "mark_move_next_str(", 19) == 0) {
        return _uscript_mark_move_next_str;
    } else if (strncmp(sig, "mark_move_next_str_ex(", 22) == 0) {
        return _uscript_mark_move_next_str_ex;
    } else if (strncmp(sig, "mark_move_next_str_nudge(", 25) == 0) {
        return _uscript_mark_move_next_str_nudge;
    } else if (strncmp(sig, "mark_move_offset(", 17) == 0) {
        return _uscript_mark_move_offset;
    } else if (strncmp(sig, "mark_move_prev_re(", 18) == 0) {
        return _uscript_mark_move_prev_re;
    } else if (strncmp(sig, "mark_move_prev_re_ex(", 21) == 0) {
        return _uscript_mark_move_prev_re_ex;
    } else if (strncmp(sig, "mark_move_prev_str(", 19) == 0) {
        return _uscript_mark_move_prev_str;
    } else if (strncmp(sig, "mark_move_prev_str_ex(", 22) == 0) {
        return _uscript_mark_move_prev_str_ex;
    } else if (strncmp(sig, "mark_move_to(", 13) == 0) {
        return _uscript_mark_move_to;
    } else if (strncmp(sig, "mark_move_to_w_bline(", 21) == 0) {
        return _uscript_mark_move_to_w_bline;
    } else if (strncmp(sig, "mark_move_vert(", 15) == 0) {
        return _uscript_mark_move_vert;
    } else if (strncmp(sig, "mark_replace(", 13) == 0) {
        return _uscript_mark_replace;
    } else if (strncmp(sig, "mark_replace_between_mark(", 26) == 0) {
        return _uscript_mark_replace_between_mark;
    } else if (strncmp(sig, "mark_swap_with_mark(", 20) == 0) {
        return _uscript_mark_swap_with_mark;
    };
    return NULL;
}

