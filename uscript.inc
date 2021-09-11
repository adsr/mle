static int _uscript_func_buffer_add_mark(lua_State *L) {
    mark_t *rv;
    buffer_t *self;
    bline_t *maybe_line;
    bint_t maybe_col;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    maybe_line = (bline_t *)luaL_checkpointer(L, 2);
    maybe_col = (bint_t)luaL_checkinteger(L, 3);
    rv = buffer_add_mark(self, maybe_line, maybe_col);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushpointer(L, (void*)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_add_mark_ex(lua_State *L) {
    mark_t *rv;
    buffer_t *self;
    char letter;
    bline_t *maybe_line;
    bint_t maybe_col;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    letter = (char)luaL_checkinteger(L, 2);
    maybe_line = (bline_t *)luaL_checkpointer(L, 3);
    maybe_col = (bint_t)luaL_checkinteger(L, 4);
    rv = buffer_add_mark_ex(self, letter, maybe_line, maybe_col);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushpointer(L, (void*)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

// static int _uscript_func_buffer_add_srule(lua_State *L) {
// }

static int _uscript_func_buffer_apply_styles(lua_State *L) {
    int rv;
    buffer_t *self;
    bline_t *start_line;
    bint_t line_delta;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    start_line = (bline_t *)luaL_checkpointer(L, 2);
    line_delta = (bint_t)luaL_checkinteger(L, 3);
    rv = buffer_apply_styles(self, start_line, line_delta);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_clear(lua_State *L) {
    int rv;
    buffer_t *self;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    rv = buffer_clear(self);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_delete(lua_State *L) {
    int rv;
    buffer_t *self;
    bint_t offset;
    bint_t num_chars;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    offset = (bint_t)luaL_checkinteger(L, 2);
    num_chars = (bint_t)luaL_checkinteger(L, 3);
    rv = buffer_delete(self, offset, num_chars);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_delete_w_bline(lua_State *L) {
    int rv;
    buffer_t *self;
    bline_t *start_line;
    bint_t start_col;
    bint_t num_chars;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    start_line = (bline_t *)luaL_checkpointer(L, 2);
    start_col = (bint_t)luaL_checkinteger(L, 3);
    num_chars = (bint_t)luaL_checkinteger(L, 4);
    rv = buffer_delete_w_bline(self, start_line, start_col, num_chars);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_destroy(lua_State *L) {
    int rv;
    buffer_t *self;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    rv = buffer_destroy(self);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_destroy_mark(lua_State *L) {
    int rv;
    buffer_t *self;
    mark_t *mark;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    mark = (mark_t *)luaL_checkpointer(L, 2);
    rv = buffer_destroy_mark(self, mark);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_get(lua_State *L) {
    int rv;
    buffer_t *self;
    char *ret_data = NULL;
    bint_t ret_data_len = 0;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    rv = buffer_get(self, &ret_data, &ret_data_len);
    lua_createtable(L, 0, 3);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_data");
    lua_pushstring(L, (const char*)ret_data);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_data_len");
    lua_pushinteger(L, (lua_Integer)ret_data_len);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_get_bline(lua_State *L) {
    int rv;
    buffer_t *self;
    bint_t line_index;
    bline_t *ret_bline = NULL;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    line_index = (bint_t)luaL_checkinteger(L, 2);
    rv = buffer_get_bline(self, line_index, &ret_bline);
    lua_createtable(L, 0, 2);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_bline");
    lua_pushpointer(L, (void*)ret_bline);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_get_bline_col(lua_State *L) {
    int rv;
    buffer_t *self;
    bint_t offset;
    bline_t *ret_bline = NULL;
    bint_t ret_col = 0;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    offset = (bint_t)luaL_checkinteger(L, 2);
    rv = buffer_get_bline_col(self, offset, &ret_bline, &ret_col);
    lua_createtable(L, 0, 3);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_bline");
    lua_pushpointer(L, (void*)ret_bline);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_col");
    lua_pushinteger(L, (lua_Integer)ret_col);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_get_bline_w_hint(lua_State *L) {
    int rv;
    buffer_t *self;
    bint_t line_index;
    bline_t *opt_hint;
    bline_t *ret_bline = NULL;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    line_index = (bint_t)luaL_checkinteger(L, 2);
    opt_hint = (bline_t *)luaL_optpointer(L, 3, NULL);
    rv = buffer_get_bline_w_hint(self, line_index, opt_hint, &ret_bline);
    lua_createtable(L, 0, 2);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_bline");
    lua_pushpointer(L, (void*)ret_bline);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_get_lettered_mark(lua_State *L) {
    int rv;
    buffer_t *self;
    char letter;
    mark_t *ret_mark = NULL;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    letter = (char)luaL_checkinteger(L, 2);
    rv = buffer_get_lettered_mark(self, letter, &ret_mark);
    lua_createtable(L, 0, 2);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_mark");
    lua_pushpointer(L, (void*)ret_mark);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_get_offset(lua_State *L) {
    int rv;
    buffer_t *self;
    bline_t *bline;
    bint_t col;
    bint_t ret_offset = 0;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    bline = (bline_t *)luaL_checkpointer(L, 2);
    col = (bint_t)luaL_checkinteger(L, 3);
    rv = buffer_get_offset(self, bline, col, &ret_offset);
    lua_createtable(L, 0, 2);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_offset");
    lua_pushinteger(L, (lua_Integer)ret_offset);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_insert(lua_State *L) {
    int rv;
    buffer_t *self;
    bint_t offset;
    char *data;
    bint_t data_len;
    bint_t optret_num_chars = 0;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    offset = (bint_t)luaL_checkinteger(L, 2);
    data = (char *)luaL_checkstring(L, 3);
    data_len = (bint_t)luaL_checkinteger(L, 4);
    rv = buffer_insert(self, offset, data, data_len, &optret_num_chars);
    lua_createtable(L, 0, 2);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "optret_num_chars");
    lua_pushinteger(L, (lua_Integer)optret_num_chars);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_insert_w_bline(lua_State *L) {
    int rv;
    buffer_t *self;
    bline_t *start_line;
    bint_t start_col;
    char *data;
    bint_t data_len;
    bint_t optret_num_chars = 0;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    start_line = (bline_t *)luaL_checkpointer(L, 2);
    start_col = (bint_t)luaL_checkinteger(L, 3);
    data = (char *)luaL_checkstring(L, 4);
    data_len = (bint_t)luaL_checkinteger(L, 5);
    rv = buffer_insert_w_bline(self, start_line, start_col, data, data_len, &optret_num_chars);
    lua_createtable(L, 0, 2);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "optret_num_chars");
    lua_pushinteger(L, (lua_Integer)optret_num_chars);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_new(lua_State *L) {
    buffer_t *rv;
    rv = buffer_new();
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushpointer(L, (void*)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_new_open(lua_State *L) {
    buffer_t *rv;
    char *path;
    path = (char *)luaL_checkstring(L, 1);
    rv = buffer_new_open(path);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushpointer(L, (void*)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_open(lua_State *L) {
    int rv;
    buffer_t *self;
    char *path;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    path = (char *)luaL_checkstring(L, 2);
    rv = buffer_open(self, path);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_redo(lua_State *L) {
    int rv;
    buffer_t *self;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    rv = buffer_redo(self);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_redo_action_group(lua_State *L) {
    int rv;
    buffer_t *self;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    rv = buffer_redo_action_group(self);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_register_append(lua_State *L) {
    int rv;
    buffer_t *self;
    char reg;
    char *data;
    size_t data_len;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    reg = (char)luaL_checkinteger(L, 2);
    data = (char *)luaL_checkstring(L, 3);
    data_len = (size_t)luaL_checkinteger(L, 4);
    rv = buffer_register_append(self, reg, data, data_len);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_register_clear(lua_State *L) {
    int rv;
    buffer_t *self;
    char reg;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    reg = (char)luaL_checkinteger(L, 2);
    rv = buffer_register_clear(self, reg);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_register_get(lua_State *L) {
    int rv;
    buffer_t *self;
    char reg;
    int dup;
    char *ret_data = NULL;
    size_t ret_data_len = 0;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    reg = (char)luaL_checkinteger(L, 2);
    dup = (int)luaL_checkinteger(L, 3);
    rv = buffer_register_get(self, reg, dup, &ret_data, &ret_data_len);
    lua_createtable(L, 0, 3);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_data");
    lua_pushstring(L, (const char*)ret_data);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_data_len");
    lua_pushpointer(L, (void*)ret_data_len);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_register_prepend(lua_State *L) {
    int rv;
    buffer_t *self;
    char reg;
    char *data;
    size_t data_len;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    reg = (char)luaL_checkinteger(L, 2);
    data = (char *)luaL_checkstring(L, 3);
    data_len = (size_t)luaL_checkinteger(L, 4);
    rv = buffer_register_prepend(self, reg, data, data_len);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_register_set(lua_State *L) {
    int rv;
    buffer_t *self;
    char reg;
    char *data;
    size_t data_len;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    reg = (char)luaL_checkinteger(L, 2);
    data = (char *)luaL_checkstring(L, 3);
    data_len = (size_t)luaL_checkinteger(L, 4);
    rv = buffer_register_set(self, reg, data, data_len);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

// static int _uscript_func_buffer_remove_srule(lua_State *L) {
// }

static int _uscript_func_buffer_replace(lua_State *L) {
    int rv;
    buffer_t *self;
    bint_t offset;
    bint_t num_chars;
    char *data;
    bint_t data_len;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    offset = (bint_t)luaL_checkinteger(L, 2);
    num_chars = (bint_t)luaL_checkinteger(L, 3);
    data = (char *)luaL_checkstring(L, 4);
    data_len = (bint_t)luaL_checkinteger(L, 5);
    rv = buffer_replace(self, offset, num_chars, data, data_len);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_replace_w_bline(lua_State *L) {
    int rv;
    buffer_t *self;
    bline_t *start_line;
    bint_t start_col;
    bint_t num_chars;
    char *data;
    bint_t data_len;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    start_line = (bline_t *)luaL_checkpointer(L, 2);
    start_col = (bint_t)luaL_checkinteger(L, 3);
    num_chars = (bint_t)luaL_checkinteger(L, 4);
    data = (char *)luaL_checkstring(L, 5);
    data_len = (bint_t)luaL_checkinteger(L, 6);
    rv = buffer_replace_w_bline(self, start_line, start_col, num_chars, data, data_len);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_save(lua_State *L) {
    int rv;
    buffer_t *self;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    rv = buffer_save(self);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_save_as(lua_State *L) {
    int rv;
    buffer_t *self;
    char *path;
    bint_t optret_nbytes = 0;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    path = (char *)luaL_checkstring(L, 2);
    rv = buffer_save_as(self, path, &optret_nbytes);
    lua_createtable(L, 0, 2);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "optret_nbytes");
    lua_pushinteger(L, (lua_Integer)optret_nbytes);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_set(lua_State *L) {
    int rv;
    buffer_t *self;
    char *data;
    bint_t data_len;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    data = (char *)luaL_checkstring(L, 2);
    data_len = (bint_t)luaL_checkinteger(L, 3);
    rv = buffer_set(self, data, data_len);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_set_action_group_ptr(lua_State *L) {
    int rv;
    buffer_t *self;
    int *action_group;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    action_group = (int *)luaL_checkinteger(L, 2);
    rv = buffer_set_action_group_ptr(self, action_group);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

// static int _uscript_func_buffer_set_callback(lua_State *L) {
// }

static int _uscript_func_buffer_set_mmapped(lua_State *L) {
    int rv;
    buffer_t *self;
    char *data;
    bint_t data_len;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    data = (char *)luaL_checkstring(L, 2);
    data_len = (bint_t)luaL_checkinteger(L, 3);
    rv = buffer_set_mmapped(self, data, data_len);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_set_styles_enabled(lua_State *L) {
    int rv;
    buffer_t *self;
    int is_enabled;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    is_enabled = (int)luaL_checkinteger(L, 2);
    rv = buffer_set_styles_enabled(self, is_enabled);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_set_tab_width(lua_State *L) {
    int rv;
    buffer_t *self;
    int tab_width;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    tab_width = (int)luaL_checkinteger(L, 2);
    rv = buffer_set_tab_width(self, tab_width);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_substr(lua_State *L) {
    int rv;
    buffer_t *self;
    bline_t *start_line;
    bint_t start_col;
    bline_t *end_line;
    bint_t end_col;
    char *ret_data = NULL;
    bint_t ret_data_len = 0;
    bint_t ret_nchars = 0;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    start_line = (bline_t *)luaL_checkpointer(L, 2);
    start_col = (bint_t)luaL_checkinteger(L, 3);
    end_line = (bline_t *)luaL_checkpointer(L, 4);
    end_col = (bint_t)luaL_checkinteger(L, 5);
    rv = buffer_substr(self, start_line, start_col, end_line, end_col, &ret_data, &ret_data_len, &ret_nchars);
    lua_createtable(L, 0, 4);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_data");
    lua_pushstring(L, (const char*)ret_data);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_data_len");
    lua_pushinteger(L, (lua_Integer)ret_data_len);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_nchars");
    lua_pushinteger(L, (lua_Integer)ret_nchars);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_undo(lua_State *L) {
    int rv;
    buffer_t *self;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    rv = buffer_undo(self);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_undo_action_group(lua_State *L) {
    int rv;
    buffer_t *self;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    rv = buffer_undo_action_group(self);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_buffer_write_to_fd(lua_State *L) {
    int rv;
    buffer_t *self;
    int fd;
    size_t optret_nbytes = 0;
    self = (buffer_t *)luaL_checkpointer(L, 1);
    fd = (int)luaL_checkinteger(L, 2);
    rv = buffer_write_to_fd(self, fd, &optret_nbytes);
    lua_createtable(L, 0, 2);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "optret_nbytes");
    lua_pushpointer(L, (void*)optret_nbytes);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

// static int _uscript_func_buffer_write_to_file(lua_State *L) {
// }

static int _uscript_func_bview_add_cursor(lua_State *L) {
    int rv;
    bview_t *self;
    bline_t *opt_bline;
    bint_t opt_col;
    cursor_t *optret_cursor = NULL;
    self = (bview_t *)luaL_checkpointer(L, 1);
    opt_bline = (bline_t *)luaL_optpointer(L, 2, NULL);
    opt_col = (bint_t)luaL_optinteger(L, 3, 0);
    rv = bview_add_cursor(self, opt_bline, opt_col, &optret_cursor);
    lua_createtable(L, 0, 2);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "optret_cursor");
    lua_pushpointer(L, (void*)optret_cursor);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_bview_add_cursor_asleep(lua_State *L) {
    int rv;
    bview_t *self;
    bline_t *opt_bline;
    bint_t opt_col;
    cursor_t *optret_cursor = NULL;
    self = (bview_t *)luaL_checkpointer(L, 1);
    opt_bline = (bline_t *)luaL_optpointer(L, 2, NULL);
    opt_col = (bint_t)luaL_optinteger(L, 3, 0);
    rv = bview_add_cursor_asleep(self, opt_bline, opt_col, &optret_cursor);
    lua_createtable(L, 0, 2);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "optret_cursor");
    lua_pushpointer(L, (void*)optret_cursor);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_bview_center_viewport_y(lua_State *L) {
    int rv;
    bview_t *self;
    self = (bview_t *)luaL_checkpointer(L, 1);
    rv = bview_center_viewport_y(self);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_bview_destroy(lua_State *L) {
    int rv;
    bview_t *self;
    self = (bview_t *)luaL_checkpointer(L, 1);
    rv = bview_destroy(self);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_bview_draw(lua_State *L) {
    int rv;
    bview_t *self;
    self = (bview_t *)luaL_checkpointer(L, 1);
    rv = bview_draw(self);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_bview_draw_cursor(lua_State *L) {
    int rv;
    bview_t *self;
    int set_real_cursor;
    self = (bview_t *)luaL_checkpointer(L, 1);
    set_real_cursor = (int)luaL_checkinteger(L, 2);
    rv = bview_draw_cursor(self, set_real_cursor);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_bview_get_active_cursor_count(lua_State *L) {
    int rv;
    bview_t *self;
    self = (bview_t *)luaL_checkpointer(L, 1);
    rv = bview_get_active_cursor_count(self);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_bview_get_split_root(lua_State *L) {
    bview_t *rv;
    bview_t *self;
    self = (bview_t *)luaL_checkpointer(L, 1);
    rv = bview_get_split_root(self);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushpointer(L, (void*)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_bview_max_viewport_y(lua_State *L) {
    int rv;
    bview_t *self;
    self = (bview_t *)luaL_checkpointer(L, 1);
    rv = bview_max_viewport_y(self);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_bview_new(lua_State *L) {
    bview_t *rv;
    editor_t *editor;
    int type;
    char *opt_path;
    int opt_path_len;
    buffer_t *opt_buffer;
    editor = (editor_t *)luaL_checkpointer(L, 1);
    type = (int)luaL_checkinteger(L, 2);
    opt_path = (char *)luaL_optstring(L, 3, NULL);
    opt_path_len = (int)luaL_optinteger(L, 4, 0);
    opt_buffer = (buffer_t *)luaL_optpointer(L, 5, NULL);
    rv = bview_new(editor, type, opt_path, opt_path_len, opt_buffer);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushpointer(L, (void*)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_bview_open(lua_State *L) {
    int rv;
    bview_t *self;
    char *path;
    int path_len;
    self = (bview_t *)luaL_checkpointer(L, 1);
    path = (char *)luaL_checkstring(L, 2);
    path_len = (int)luaL_checkinteger(L, 3);
    rv = bview_open(self, path, path_len);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

// static int _uscript_func_bview_pop_kmap(lua_State *L) {
// }

// static int _uscript_func_bview_push_kmap(lua_State *L) {
// }

static int _uscript_func_bview_rectify_viewport(lua_State *L) {
    int rv;
    bview_t *self;
    self = (bview_t *)luaL_checkpointer(L, 1);
    rv = bview_rectify_viewport(self);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_bview_remove_cursor(lua_State *L) {
    int rv;
    bview_t *self;
    cursor_t *cursor;
    self = (bview_t *)luaL_checkpointer(L, 1);
    cursor = (cursor_t *)luaL_checkpointer(L, 2);
    rv = bview_remove_cursor(self, cursor);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_bview_remove_cursors_except(lua_State *L) {
    int rv;
    bview_t *self;
    cursor_t *one;
    self = (bview_t *)luaL_checkpointer(L, 1);
    one = (cursor_t *)luaL_checkpointer(L, 2);
    rv = bview_remove_cursors_except(self, one);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_bview_resize(lua_State *L) {
    int rv;
    bview_t *self;
    int x;
    int y;
    int w;
    int h;
    self = (bview_t *)luaL_checkpointer(L, 1);
    x = (int)luaL_checkinteger(L, 2);
    y = (int)luaL_checkinteger(L, 3);
    w = (int)luaL_checkinteger(L, 4);
    h = (int)luaL_checkinteger(L, 5);
    rv = bview_resize(self, x, y, w, h);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_bview_set_syntax(lua_State *L) {
    int rv;
    bview_t *self;
    char *opt_syntax;
    self = (bview_t *)luaL_checkpointer(L, 1);
    opt_syntax = (char *)luaL_optstring(L, 2, NULL);
    rv = bview_set_syntax(self, opt_syntax);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_bview_set_viewport_y(lua_State *L) {
    int rv;
    bview_t *self;
    bint_t y;
    int do_rectify;
    self = (bview_t *)luaL_checkpointer(L, 1);
    y = (bint_t)luaL_checkinteger(L, 2);
    do_rectify = (int)luaL_checkinteger(L, 3);
    rv = bview_set_viewport_y(self, y, do_rectify);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_bview_split(lua_State *L) {
    int rv;
    bview_t *self;
    int is_vertical;
    float factor;
    bview_t *optret_bview = NULL;
    self = (bview_t *)luaL_checkpointer(L, 1);
    is_vertical = (int)luaL_checkinteger(L, 2);
    factor = (float)luaL_checknumber(L, 3);
    rv = bview_split(self, is_vertical, factor, &optret_bview);
    lua_createtable(L, 0, 2);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "optret_bview");
    lua_pushpointer(L, (void*)optret_bview);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_bview_wake_sleeping_cursors(lua_State *L) {
    int rv;
    bview_t *self;
    self = (bview_t *)luaL_checkpointer(L, 1);
    rv = bview_wake_sleeping_cursors(self);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_bview_zero_viewport_y(lua_State *L) {
    int rv;
    bview_t *self;
    self = (bview_t *)luaL_checkpointer(L, 1);
    rv = bview_zero_viewport_y(self);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_cursor_clone(lua_State *L) {
    int rv;
    cursor_t *cursor;
    int use_srules;
    cursor_t *ret_clone = NULL;
    cursor = (cursor_t *)luaL_checkpointer(L, 1);
    use_srules = (int)luaL_checkinteger(L, 2);
    rv = cursor_clone(cursor, use_srules, &ret_clone);
    lua_createtable(L, 0, 2);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_clone");
    lua_pushpointer(L, (void*)ret_clone);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_cursor_cut_copy(lua_State *L) {
    int rv;
    cursor_t *cursor;
    int is_cut;
    int use_srules;
    int append;
    cursor = (cursor_t *)luaL_checkpointer(L, 1);
    is_cut = (int)luaL_checkinteger(L, 2);
    use_srules = (int)luaL_checkinteger(L, 3);
    append = (int)luaL_checkinteger(L, 4);
    rv = cursor_cut_copy(cursor, is_cut, use_srules, append);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_cursor_destroy(lua_State *L) {
    int rv;
    cursor_t *cursor;
    cursor = (cursor_t *)luaL_checkpointer(L, 1);
    rv = cursor_destroy(cursor);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_cursor_drop_anchor(lua_State *L) {
    int rv;
    cursor_t *cursor;
    int use_srules;
    cursor = (cursor_t *)luaL_checkpointer(L, 1);
    use_srules = (int)luaL_checkinteger(L, 2);
    rv = cursor_drop_anchor(cursor, use_srules);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_cursor_get_anchor(lua_State *L) {
    int rv;
    cursor_t *cursor;
    mark_t *ret_anchor = NULL;
    cursor = (cursor_t *)luaL_checkpointer(L, 1);
    rv = cursor_get_anchor(cursor, &ret_anchor);
    lua_createtable(L, 0, 2);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_anchor");
    lua_pushpointer(L, (void*)ret_anchor);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_cursor_get_lo_hi(lua_State *L) {
    int rv;
    cursor_t *cursor;
    mark_t *ret_lo = NULL;
    mark_t *ret_hi = NULL;
    cursor = (cursor_t *)luaL_checkpointer(L, 1);
    rv = cursor_get_lo_hi(cursor, &ret_lo, &ret_hi);
    lua_createtable(L, 0, 3);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_lo");
    lua_pushpointer(L, (void*)ret_lo);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_hi");
    lua_pushpointer(L, (void*)ret_hi);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_cursor_get_mark(lua_State *L) {
    int rv;
    cursor_t *cursor;
    mark_t *ret_mark = NULL;
    cursor = (cursor_t *)luaL_checkpointer(L, 1);
    rv = cursor_get_mark(cursor, &ret_mark);
    lua_createtable(L, 0, 2);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_mark");
    lua_pushpointer(L, (void*)ret_mark);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_cursor_lift_anchor(lua_State *L) {
    int rv;
    cursor_t *cursor;
    cursor = (cursor_t *)luaL_checkpointer(L, 1);
    rv = cursor_lift_anchor(cursor);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_cursor_replace(lua_State *L) {
    int rv;
    cursor_t *cursor;
    int interactive;
    char *opt_regex;
    char *opt_replacement;
    cursor = (cursor_t *)luaL_checkpointer(L, 1);
    interactive = (int)luaL_checkinteger(L, 2);
    opt_regex = (char *)luaL_optstring(L, 3, NULL);
    opt_replacement = (char *)luaL_optstring(L, 4, NULL);
    rv = cursor_replace(cursor, interactive, opt_regex, opt_replacement);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_cursor_select_between(lua_State *L) {
    int rv;
    cursor_t *cursor;
    mark_t *a;
    mark_t *b;
    int use_srules;
    cursor = (cursor_t *)luaL_checkpointer(L, 1);
    a = (mark_t *)luaL_checkpointer(L, 2);
    b = (mark_t *)luaL_checkpointer(L, 3);
    use_srules = (int)luaL_checkinteger(L, 4);
    rv = cursor_select_between(cursor, a, b, use_srules);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_cursor_select_by(lua_State *L) {
    int rv;
    cursor_t *cursor;
    const char *strat;
    int use_srules;
    cursor = (cursor_t *)luaL_checkpointer(L, 1);
    strat = (const char *)luaL_checkstring(L, 2);
    use_srules = (int)luaL_checkinteger(L, 3);
    rv = cursor_select_by(cursor, strat, use_srules);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_cursor_select_by_bracket(lua_State *L) {
    int rv;
    cursor_t *cursor;
    int use_srules;
    cursor = (cursor_t *)luaL_checkpointer(L, 1);
    use_srules = (int)luaL_checkinteger(L, 2);
    rv = cursor_select_by_bracket(cursor, use_srules);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_cursor_select_by_string(lua_State *L) {
    int rv;
    cursor_t *cursor;
    int use_srules;
    cursor = (cursor_t *)luaL_checkpointer(L, 1);
    use_srules = (int)luaL_checkinteger(L, 2);
    rv = cursor_select_by_string(cursor, use_srules);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_cursor_select_by_word(lua_State *L) {
    int rv;
    cursor_t *cursor;
    int use_srules;
    cursor = (cursor_t *)luaL_checkpointer(L, 1);
    use_srules = (int)luaL_checkinteger(L, 2);
    rv = cursor_select_by_word(cursor, use_srules);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_cursor_select_by_word_back(lua_State *L) {
    int rv;
    cursor_t *cursor;
    int use_srules;
    cursor = (cursor_t *)luaL_checkpointer(L, 1);
    use_srules = (int)luaL_checkinteger(L, 2);
    rv = cursor_select_by_word_back(cursor, use_srules);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_cursor_select_by_word_forward(lua_State *L) {
    int rv;
    cursor_t *cursor;
    int use_srules;
    cursor = (cursor_t *)luaL_checkpointer(L, 1);
    use_srules = (int)luaL_checkinteger(L, 2);
    rv = cursor_select_by_word_forward(cursor, use_srules);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_cursor_toggle_anchor(lua_State *L) {
    int rv;
    cursor_t *cursor;
    int use_srules;
    cursor = (cursor_t *)luaL_checkpointer(L, 1);
    use_srules = (int)luaL_checkinteger(L, 2);
    rv = cursor_toggle_anchor(cursor, use_srules);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_cursor_uncut(lua_State *L) {
    int rv;
    cursor_t *cursor;
    cursor = (cursor_t *)luaL_checkpointer(L, 1);
    rv = cursor_uncut(cursor);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_editor_bview_edit_count(lua_State *L) {
    int rv;
    editor_t *editor;
    editor = (editor_t *)luaL_checkpointer(L, 1);
    rv = editor_bview_edit_count(editor);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_editor_close_bview(lua_State *L) {
    int rv;
    editor_t *editor;
    bview_t *bview;
    int optret_num_closed = 0;
    editor = (editor_t *)luaL_checkpointer(L, 1);
    bview = (bview_t *)luaL_checkpointer(L, 2);
    rv = editor_close_bview(editor, bview, &optret_num_closed);
    lua_createtable(L, 0, 2);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "optret_num_closed");
    lua_pushinteger(L, (lua_Integer)optret_num_closed);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_editor_count_bviews_by_buffer(lua_State *L) {
    int rv;
    editor_t *editor;
    buffer_t *buffer;
    editor = (editor_t *)luaL_checkpointer(L, 1);
    buffer = (buffer_t *)luaL_checkpointer(L, 2);
    rv = editor_count_bviews_by_buffer(editor, buffer);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_editor_destroy_observer(lua_State *L) {
    int rv;
    editor_t *editor;
    observer_t *observer;
    editor = (editor_t *)luaL_checkpointer(L, 1);
    observer = (observer_t *)luaL_checkpointer(L, 2);
    rv = editor_destroy_observer(editor, observer);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_editor_display(lua_State *L) {
    int rv;
    editor_t *editor;
    editor = (editor_t *)luaL_checkpointer(L, 1);
    rv = editor_display(editor);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_editor_force_redraw(lua_State *L) {
    int rv;
    editor_t *editor;
    editor = (editor_t *)luaL_checkpointer(L, 1);
    rv = editor_force_redraw(editor);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

// static int _uscript_func_editor_get_input(lua_State *L) {
// }

// static int _uscript_func_editor_menu(lua_State *L) {
// }

static int _uscript_func_editor_notify_observers(lua_State *L) {
    int rv;
    editor_t *editor;
    char *event_name;
    void *event_data;
    editor = (editor_t *)luaL_checkpointer(L, 1);
    event_name = (char *)luaL_checkstring(L, 2);
    event_data = (void *)luaL_checkpointer(L, 3);
    rv = editor_notify_observers(editor, event_name, event_data);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_editor_open_bview(lua_State *L) {
    int rv;
    editor_t *editor;
    bview_t *opt_parent;
    int type;
    char *opt_path;
    int opt_path_len;
    int make_active;
    bint_t linenum;
    int skip_resize;
    buffer_t *opt_buffer;
    bview_t *optret_bview = NULL;
    editor = (editor_t *)luaL_checkpointer(L, 1);
    opt_parent = (bview_t *)luaL_optpointer(L, 2, NULL);
    type = (int)luaL_checkinteger(L, 3);
    opt_path = (char *)luaL_optstring(L, 4, NULL);
    opt_path_len = (int)luaL_optinteger(L, 5, 0);
    make_active = (int)luaL_checkinteger(L, 6);
    linenum = (bint_t)luaL_checkinteger(L, 7);
    skip_resize = (int)luaL_checkinteger(L, 8);
    opt_buffer = (buffer_t *)luaL_optpointer(L, 9, NULL);
    rv = editor_open_bview(editor, opt_parent, type, opt_path, opt_path_len, make_active, linenum, skip_resize, opt_buffer, &optret_bview);
    lua_createtable(L, 0, 2);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "optret_bview");
    lua_pushpointer(L, (void*)optret_bview);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

// static int _uscript_func_editor_prompt(lua_State *L) {
// }

// static int _uscript_func_editor_register_cmd(lua_State *L) {
// }

// static int _uscript_func_editor_register_observer(lua_State *L) {
// }

static int _uscript_func_editor_set_active(lua_State *L) {
    int rv;
    editor_t *editor;
    bview_t *bview;
    editor = (editor_t *)luaL_checkpointer(L, 1);
    bview = (bview_t *)luaL_checkpointer(L, 2);
    rv = editor_set_active(editor, bview);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_clone(lua_State *L) {
    int rv;
    mark_t *self;
    mark_t *ret_mark = NULL;
    self = (mark_t *)luaL_checkpointer(L, 1);
    rv = mark_clone(self, &ret_mark);
    lua_createtable(L, 0, 2);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_mark");
    lua_pushpointer(L, (void*)ret_mark);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_clone_w_letter(lua_State *L) {
    int rv;
    mark_t *self;
    char letter;
    mark_t *ret_mark = NULL;
    self = (mark_t *)luaL_checkpointer(L, 1);
    letter = (char)luaL_checkinteger(L, 2);
    rv = mark_clone_w_letter(self, letter, &ret_mark);
    lua_createtable(L, 0, 2);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_mark");
    lua_pushpointer(L, (void*)ret_mark);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_delete_after(lua_State *L) {
    int rv;
    mark_t *self;
    bint_t num_chars;
    self = (mark_t *)luaL_checkpointer(L, 1);
    num_chars = (bint_t)luaL_checkinteger(L, 2);
    rv = mark_delete_after(self, num_chars);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_delete_before(lua_State *L) {
    int rv;
    mark_t *self;
    bint_t num_chars;
    self = (mark_t *)luaL_checkpointer(L, 1);
    num_chars = (bint_t)luaL_checkinteger(L, 2);
    rv = mark_delete_before(self, num_chars);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_delete_between_mark(lua_State *L) {
    int rv;
    mark_t *self;
    mark_t *other;
    self = (mark_t *)luaL_checkpointer(L, 1);
    other = (mark_t *)luaL_checkpointer(L, 2);
    rv = mark_delete_between_mark(self, other);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_destroy(lua_State *L) {
    int rv;
    mark_t *self;
    self = (mark_t *)luaL_checkpointer(L, 1);
    rv = mark_destroy(self);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_find_bracket_pair(lua_State *L) {
    int rv;
    mark_t *self;
    bint_t max_chars;
    bline_t *ret_line = NULL;
    bint_t ret_col = 0;
    bint_t ret_brkt = 0;
    self = (mark_t *)luaL_checkpointer(L, 1);
    max_chars = (bint_t)luaL_checkinteger(L, 2);
    rv = mark_find_bracket_pair(self, max_chars, &ret_line, &ret_col, &ret_brkt);
    lua_createtable(L, 0, 4);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_line");
    lua_pushpointer(L, (void*)ret_line);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_col");
    lua_pushinteger(L, (lua_Integer)ret_col);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_brkt");
    lua_pushinteger(L, (lua_Integer)ret_brkt);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_find_bracket_top(lua_State *L) {
    int rv;
    mark_t *self;
    bint_t max_chars;
    bline_t *ret_line = NULL;
    bint_t ret_col = 0;
    bint_t ret_brkt = 0;
    self = (mark_t *)luaL_checkpointer(L, 1);
    max_chars = (bint_t)luaL_checkinteger(L, 2);
    rv = mark_find_bracket_top(self, max_chars, &ret_line, &ret_col, &ret_brkt);
    lua_createtable(L, 0, 4);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_line");
    lua_pushpointer(L, (void*)ret_line);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_col");
    lua_pushinteger(L, (lua_Integer)ret_col);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_brkt");
    lua_pushinteger(L, (lua_Integer)ret_brkt);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_find_next_re(lua_State *L) {
    int rv;
    mark_t *self;
    char *re;
    bint_t re_len;
    bline_t *ret_line = NULL;
    bint_t ret_col = 0;
    bint_t ret_num_chars = 0;
    self = (mark_t *)luaL_checkpointer(L, 1);
    re = (char *)luaL_checkstring(L, 2);
    re_len = (bint_t)luaL_checkinteger(L, 3);
    rv = mark_find_next_re(self, re, re_len, &ret_line, &ret_col, &ret_num_chars);
    lua_createtable(L, 0, 4);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_line");
    lua_pushpointer(L, (void*)ret_line);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_col");
    lua_pushinteger(L, (lua_Integer)ret_col);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_num_chars");
    lua_pushinteger(L, (lua_Integer)ret_num_chars);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_find_next_str(lua_State *L) {
    int rv;
    mark_t *self;
    char *str;
    bint_t str_len;
    bline_t *ret_line = NULL;
    bint_t ret_col = 0;
    bint_t ret_num_chars = 0;
    self = (mark_t *)luaL_checkpointer(L, 1);
    str = (char *)luaL_checkstring(L, 2);
    str_len = (bint_t)luaL_checkinteger(L, 3);
    rv = mark_find_next_str(self, str, str_len, &ret_line, &ret_col, &ret_num_chars);
    lua_createtable(L, 0, 4);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_line");
    lua_pushpointer(L, (void*)ret_line);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_col");
    lua_pushinteger(L, (lua_Integer)ret_col);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_num_chars");
    lua_pushinteger(L, (lua_Integer)ret_num_chars);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_find_prev_re(lua_State *L) {
    int rv;
    mark_t *self;
    char *re;
    bint_t re_len;
    bline_t *ret_line = NULL;
    bint_t ret_col = 0;
    bint_t ret_num_chars = 0;
    self = (mark_t *)luaL_checkpointer(L, 1);
    re = (char *)luaL_checkstring(L, 2);
    re_len = (bint_t)luaL_checkinteger(L, 3);
    rv = mark_find_prev_re(self, re, re_len, &ret_line, &ret_col, &ret_num_chars);
    lua_createtable(L, 0, 4);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_line");
    lua_pushpointer(L, (void*)ret_line);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_col");
    lua_pushinteger(L, (lua_Integer)ret_col);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_num_chars");
    lua_pushinteger(L, (lua_Integer)ret_num_chars);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_find_prev_str(lua_State *L) {
    int rv;
    mark_t *self;
    char *str;
    bint_t str_len;
    bline_t *ret_line = NULL;
    bint_t ret_col = 0;
    bint_t ret_num_chars = 0;
    self = (mark_t *)luaL_checkpointer(L, 1);
    str = (char *)luaL_checkstring(L, 2);
    str_len = (bint_t)luaL_checkinteger(L, 3);
    rv = mark_find_prev_str(self, str, str_len, &ret_line, &ret_col, &ret_num_chars);
    lua_createtable(L, 0, 4);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_line");
    lua_pushpointer(L, (void*)ret_line);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_col");
    lua_pushinteger(L, (lua_Integer)ret_col);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_num_chars");
    lua_pushinteger(L, (lua_Integer)ret_num_chars);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_get_between_mark(lua_State *L) {
    int rv;
    mark_t *self;
    mark_t *other;
    char *ret_str = NULL;
    bint_t ret_str_len = 0;
    self = (mark_t *)luaL_checkpointer(L, 1);
    other = (mark_t *)luaL_checkpointer(L, 2);
    rv = mark_get_between_mark(self, other, &ret_str, &ret_str_len);
    lua_createtable(L, 0, 3);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_str");
    lua_pushstring(L, (const char*)ret_str);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_str_len");
    lua_pushinteger(L, (lua_Integer)ret_str_len);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_get_char_after(lua_State *L) {
    int rv;
    mark_t *self;
    uint32_t ret_char = 0;
    self = (mark_t *)luaL_checkpointer(L, 1);
    rv = mark_get_char_after(self, &ret_char);
    lua_createtable(L, 0, 2);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_char");
    lua_pushinteger(L, (lua_Integer)ret_char);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_get_char_before(lua_State *L) {
    int rv;
    mark_t *self;
    uint32_t ret_char = 0;
    self = (mark_t *)luaL_checkpointer(L, 1);
    rv = mark_get_char_before(self, &ret_char);
    lua_createtable(L, 0, 2);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_char");
    lua_pushinteger(L, (lua_Integer)ret_char);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_get_nchars_between(lua_State *L) {
    int rv;
    mark_t *self;
    mark_t *other;
    bint_t ret_nchars = 0;
    self = (mark_t *)luaL_checkpointer(L, 1);
    other = (mark_t *)luaL_checkpointer(L, 2);
    rv = mark_get_nchars_between(self, other, &ret_nchars);
    lua_createtable(L, 0, 2);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_nchars");
    lua_pushinteger(L, (lua_Integer)ret_nchars);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_get_offset(lua_State *L) {
    int rv;
    mark_t *self;
    bint_t ret_offset = 0;
    self = (mark_t *)luaL_checkpointer(L, 1);
    rv = mark_get_offset(self, &ret_offset);
    lua_createtable(L, 0, 2);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "ret_offset");
    lua_pushinteger(L, (lua_Integer)ret_offset);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_insert_after(lua_State *L) {
    int rv;
    mark_t *self;
    char *data;
    bint_t data_len;
    self = (mark_t *)luaL_checkpointer(L, 1);
    data = (char *)luaL_checkstring(L, 2);
    data_len = (bint_t)luaL_checkinteger(L, 3);
    rv = mark_insert_after(self, data, data_len);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_insert_before(lua_State *L) {
    int rv;
    mark_t *self;
    char *data;
    bint_t data_len;
    self = (mark_t *)luaL_checkpointer(L, 1);
    data = (char *)luaL_checkstring(L, 2);
    data_len = (bint_t)luaL_checkinteger(L, 3);
    rv = mark_insert_before(self, data, data_len);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_is_after_col_minus_lefties(lua_State *L) {
    int rv;
    mark_t *self;
    bint_t col;
    self = (mark_t *)luaL_checkpointer(L, 1);
    col = (bint_t)luaL_checkinteger(L, 2);
    rv = mark_is_after_col_minus_lefties(self, col);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_is_at_bol(lua_State *L) {
    int rv;
    mark_t *self;
    self = (mark_t *)luaL_checkpointer(L, 1);
    rv = mark_is_at_bol(self);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_is_at_eol(lua_State *L) {
    int rv;
    mark_t *self;
    self = (mark_t *)luaL_checkpointer(L, 1);
    rv = mark_is_at_eol(self);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_is_at_word_bound(lua_State *L) {
    int rv;
    mark_t *self;
    int side;
    self = (mark_t *)luaL_checkpointer(L, 1);
    side = (int)luaL_checkinteger(L, 2);
    rv = mark_is_at_word_bound(self, side);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_is_between(lua_State *L) {
    int rv;
    mark_t *self;
    mark_t *ma;
    mark_t *mb;
    self = (mark_t *)luaL_checkpointer(L, 1);
    ma = (mark_t *)luaL_checkpointer(L, 2);
    mb = (mark_t *)luaL_checkpointer(L, 3);
    rv = mark_is_between(self, ma, mb);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_is_eq(lua_State *L) {
    int rv;
    mark_t *self;
    mark_t *other;
    self = (mark_t *)luaL_checkpointer(L, 1);
    other = (mark_t *)luaL_checkpointer(L, 2);
    rv = mark_is_eq(self, other);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_is_gt(lua_State *L) {
    int rv;
    mark_t *self;
    mark_t *other;
    self = (mark_t *)luaL_checkpointer(L, 1);
    other = (mark_t *)luaL_checkpointer(L, 2);
    rv = mark_is_gt(self, other);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_is_gte(lua_State *L) {
    int rv;
    mark_t *self;
    mark_t *other;
    self = (mark_t *)luaL_checkpointer(L, 1);
    other = (mark_t *)luaL_checkpointer(L, 2);
    rv = mark_is_gte(self, other);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_is_lt(lua_State *L) {
    int rv;
    mark_t *self;
    mark_t *other;
    self = (mark_t *)luaL_checkpointer(L, 1);
    other = (mark_t *)luaL_checkpointer(L, 2);
    rv = mark_is_lt(self, other);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_is_lte(lua_State *L) {
    int rv;
    mark_t *self;
    mark_t *other;
    self = (mark_t *)luaL_checkpointer(L, 1);
    other = (mark_t *)luaL_checkpointer(L, 2);
    rv = mark_is_lte(self, other);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_join(lua_State *L) {
    int rv;
    mark_t *self;
    mark_t *other;
    self = (mark_t *)luaL_checkpointer(L, 1);
    other = (mark_t *)luaL_checkpointer(L, 2);
    rv = mark_join(self, other);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_move_beginning(lua_State *L) {
    int rv;
    mark_t *self;
    self = (mark_t *)luaL_checkpointer(L, 1);
    rv = mark_move_beginning(self);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_move_bol(lua_State *L) {
    int rv;
    mark_t *self;
    self = (mark_t *)luaL_checkpointer(L, 1);
    rv = mark_move_bol(self);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_move_bracket_pair(lua_State *L) {
    int rv;
    mark_t *self;
    bint_t max_chars;
    self = (mark_t *)luaL_checkpointer(L, 1);
    max_chars = (bint_t)luaL_checkinteger(L, 2);
    rv = mark_move_bracket_pair(self, max_chars);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_move_bracket_pair_ex(lua_State *L) {
    int rv;
    mark_t *self;
    bint_t max_chars;
    bline_t *optret_line = NULL;
    bint_t optret_col = 0;
    bint_t optret_num_chars = 0;
    self = (mark_t *)luaL_checkpointer(L, 1);
    max_chars = (bint_t)luaL_checkinteger(L, 2);
    rv = mark_move_bracket_pair_ex(self, max_chars, &optret_line, &optret_col, &optret_num_chars);
    lua_createtable(L, 0, 4);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "optret_line");
    lua_pushpointer(L, (void*)optret_line);
    lua_settable(L, -3);
    lua_pushstring(L, "optret_col");
    lua_pushinteger(L, (lua_Integer)optret_col);
    lua_settable(L, -3);
    lua_pushstring(L, "optret_num_chars");
    lua_pushinteger(L, (lua_Integer)optret_num_chars);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_move_bracket_top(lua_State *L) {
    int rv;
    mark_t *self;
    bint_t max_chars;
    self = (mark_t *)luaL_checkpointer(L, 1);
    max_chars = (bint_t)luaL_checkinteger(L, 2);
    rv = mark_move_bracket_top(self, max_chars);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_move_bracket_top_ex(lua_State *L) {
    int rv;
    mark_t *self;
    bint_t max_chars;
    bline_t *optret_line = NULL;
    bint_t optret_col = 0;
    bint_t optret_num_chars = 0;
    self = (mark_t *)luaL_checkpointer(L, 1);
    max_chars = (bint_t)luaL_checkinteger(L, 2);
    rv = mark_move_bracket_top_ex(self, max_chars, &optret_line, &optret_col, &optret_num_chars);
    lua_createtable(L, 0, 4);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "optret_line");
    lua_pushpointer(L, (void*)optret_line);
    lua_settable(L, -3);
    lua_pushstring(L, "optret_col");
    lua_pushinteger(L, (lua_Integer)optret_col);
    lua_settable(L, -3);
    lua_pushstring(L, "optret_num_chars");
    lua_pushinteger(L, (lua_Integer)optret_num_chars);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_move_by(lua_State *L) {
    int rv;
    mark_t *self;
    bint_t char_delta;
    self = (mark_t *)luaL_checkpointer(L, 1);
    char_delta = (bint_t)luaL_checkinteger(L, 2);
    rv = mark_move_by(self, char_delta);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_move_col(lua_State *L) {
    int rv;
    mark_t *self;
    bint_t col;
    self = (mark_t *)luaL_checkpointer(L, 1);
    col = (bint_t)luaL_checkinteger(L, 2);
    rv = mark_move_col(self, col);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_move_end(lua_State *L) {
    int rv;
    mark_t *self;
    self = (mark_t *)luaL_checkpointer(L, 1);
    rv = mark_move_end(self);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_move_eol(lua_State *L) {
    int rv;
    mark_t *self;
    self = (mark_t *)luaL_checkpointer(L, 1);
    rv = mark_move_eol(self);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_move_next_re(lua_State *L) {
    int rv;
    mark_t *self;
    char *re;
    bint_t re_len;
    self = (mark_t *)luaL_checkpointer(L, 1);
    re = (char *)luaL_checkstring(L, 2);
    re_len = (bint_t)luaL_checkinteger(L, 3);
    rv = mark_move_next_re(self, re, re_len);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_move_next_re_ex(lua_State *L) {
    int rv;
    mark_t *self;
    char *re;
    bint_t re_len;
    bline_t *optret_line = NULL;
    bint_t optret_col = 0;
    bint_t optret_num_chars = 0;
    self = (mark_t *)luaL_checkpointer(L, 1);
    re = (char *)luaL_checkstring(L, 2);
    re_len = (bint_t)luaL_checkinteger(L, 3);
    rv = mark_move_next_re_ex(self, re, re_len, &optret_line, &optret_col, &optret_num_chars);
    lua_createtable(L, 0, 4);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "optret_line");
    lua_pushpointer(L, (void*)optret_line);
    lua_settable(L, -3);
    lua_pushstring(L, "optret_col");
    lua_pushinteger(L, (lua_Integer)optret_col);
    lua_settable(L, -3);
    lua_pushstring(L, "optret_num_chars");
    lua_pushinteger(L, (lua_Integer)optret_num_chars);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_move_next_re_nudge(lua_State *L) {
    int rv;
    mark_t *self;
    char *re;
    bint_t re_len;
    self = (mark_t *)luaL_checkpointer(L, 1);
    re = (char *)luaL_checkstring(L, 2);
    re_len = (bint_t)luaL_checkinteger(L, 3);
    rv = mark_move_next_re_nudge(self, re, re_len);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_move_next_str(lua_State *L) {
    int rv;
    mark_t *self;
    char *str;
    bint_t str_len;
    self = (mark_t *)luaL_checkpointer(L, 1);
    str = (char *)luaL_checkstring(L, 2);
    str_len = (bint_t)luaL_checkinteger(L, 3);
    rv = mark_move_next_str(self, str, str_len);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_move_next_str_ex(lua_State *L) {
    int rv;
    mark_t *self;
    char *str;
    bint_t str_len;
    bline_t *optret_line = NULL;
    bint_t optret_col = 0;
    bint_t optret_num_chars = 0;
    self = (mark_t *)luaL_checkpointer(L, 1);
    str = (char *)luaL_checkstring(L, 2);
    str_len = (bint_t)luaL_checkinteger(L, 3);
    rv = mark_move_next_str_ex(self, str, str_len, &optret_line, &optret_col, &optret_num_chars);
    lua_createtable(L, 0, 4);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "optret_line");
    lua_pushpointer(L, (void*)optret_line);
    lua_settable(L, -3);
    lua_pushstring(L, "optret_col");
    lua_pushinteger(L, (lua_Integer)optret_col);
    lua_settable(L, -3);
    lua_pushstring(L, "optret_num_chars");
    lua_pushinteger(L, (lua_Integer)optret_num_chars);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_move_next_str_nudge(lua_State *L) {
    int rv;
    mark_t *self;
    char *str;
    bint_t str_len;
    self = (mark_t *)luaL_checkpointer(L, 1);
    str = (char *)luaL_checkstring(L, 2);
    str_len = (bint_t)luaL_checkinteger(L, 3);
    rv = mark_move_next_str_nudge(self, str, str_len);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_move_offset(lua_State *L) {
    int rv;
    mark_t *self;
    bint_t offset;
    self = (mark_t *)luaL_checkpointer(L, 1);
    offset = (bint_t)luaL_checkinteger(L, 2);
    rv = mark_move_offset(self, offset);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_move_prev_re(lua_State *L) {
    int rv;
    mark_t *self;
    char *re;
    bint_t re_len;
    self = (mark_t *)luaL_checkpointer(L, 1);
    re = (char *)luaL_checkstring(L, 2);
    re_len = (bint_t)luaL_checkinteger(L, 3);
    rv = mark_move_prev_re(self, re, re_len);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_move_prev_re_ex(lua_State *L) {
    int rv;
    mark_t *self;
    char *re;
    bint_t re_len;
    bline_t *optret_line = NULL;
    bint_t optret_col = 0;
    bint_t optret_num_chars = 0;
    self = (mark_t *)luaL_checkpointer(L, 1);
    re = (char *)luaL_checkstring(L, 2);
    re_len = (bint_t)luaL_checkinteger(L, 3);
    rv = mark_move_prev_re_ex(self, re, re_len, &optret_line, &optret_col, &optret_num_chars);
    lua_createtable(L, 0, 4);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "optret_line");
    lua_pushpointer(L, (void*)optret_line);
    lua_settable(L, -3);
    lua_pushstring(L, "optret_col");
    lua_pushinteger(L, (lua_Integer)optret_col);
    lua_settable(L, -3);
    lua_pushstring(L, "optret_num_chars");
    lua_pushinteger(L, (lua_Integer)optret_num_chars);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_move_prev_str(lua_State *L) {
    int rv;
    mark_t *self;
    char *str;
    bint_t str_len;
    self = (mark_t *)luaL_checkpointer(L, 1);
    str = (char *)luaL_checkstring(L, 2);
    str_len = (bint_t)luaL_checkinteger(L, 3);
    rv = mark_move_prev_str(self, str, str_len);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_move_prev_str_ex(lua_State *L) {
    int rv;
    mark_t *self;
    char *str;
    bint_t str_len;
    bline_t *optret_line = NULL;
    bint_t optret_col = 0;
    bint_t optret_num_chars = 0;
    self = (mark_t *)luaL_checkpointer(L, 1);
    str = (char *)luaL_checkstring(L, 2);
    str_len = (bint_t)luaL_checkinteger(L, 3);
    rv = mark_move_prev_str_ex(self, str, str_len, &optret_line, &optret_col, &optret_num_chars);
    lua_createtable(L, 0, 4);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushstring(L, "optret_line");
    lua_pushpointer(L, (void*)optret_line);
    lua_settable(L, -3);
    lua_pushstring(L, "optret_col");
    lua_pushinteger(L, (lua_Integer)optret_col);
    lua_settable(L, -3);
    lua_pushstring(L, "optret_num_chars");
    lua_pushinteger(L, (lua_Integer)optret_num_chars);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_move_to(lua_State *L) {
    int rv;
    mark_t *self;
    bint_t line_index;
    bint_t col;
    self = (mark_t *)luaL_checkpointer(L, 1);
    line_index = (bint_t)luaL_checkinteger(L, 2);
    col = (bint_t)luaL_checkinteger(L, 3);
    rv = mark_move_to(self, line_index, col);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_move_to_w_bline(lua_State *L) {
    int rv;
    mark_t *self;
    bline_t *bline;
    bint_t col;
    self = (mark_t *)luaL_checkpointer(L, 1);
    bline = (bline_t *)luaL_checkpointer(L, 2);
    col = (bint_t)luaL_checkinteger(L, 3);
    rv = mark_move_to_w_bline(self, bline, col);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_move_vert(lua_State *L) {
    int rv;
    mark_t *self;
    bint_t line_delta;
    self = (mark_t *)luaL_checkpointer(L, 1);
    line_delta = (bint_t)luaL_checkinteger(L, 2);
    rv = mark_move_vert(self, line_delta);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_replace(lua_State *L) {
    int rv;
    mark_t *self;
    bint_t num_chars;
    char *data;
    bint_t data_len;
    self = (mark_t *)luaL_checkpointer(L, 1);
    num_chars = (bint_t)luaL_checkinteger(L, 2);
    data = (char *)luaL_checkstring(L, 3);
    data_len = (bint_t)luaL_checkinteger(L, 4);
    rv = mark_replace(self, num_chars, data, data_len);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_replace_between_mark(lua_State *L) {
    int rv;
    mark_t *self;
    mark_t *other;
    char *data;
    bint_t data_len;
    self = (mark_t *)luaL_checkpointer(L, 1);
    other = (mark_t *)luaL_checkpointer(L, 2);
    data = (char *)luaL_checkstring(L, 3);
    data_len = (bint_t)luaL_checkinteger(L, 4);
    rv = mark_replace_between_mark(self, other, data, data_len);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

static int _uscript_func_mark_swap_with_mark(lua_State *L) {
    int rv;
    mark_t *self;
    mark_t *other;
    self = (mark_t *)luaL_checkpointer(L, 1);
    other = (mark_t *)luaL_checkpointer(L, 2);
    rv = mark_swap_with_mark(self, other);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "rv");
    lua_pushinteger(L, (lua_Integer)rv);
    lua_settable(L, -3);
    lua_pushvalue(L, -1);
    return 1;
}

// static int _uscript_func_util_escape_shell_arg(lua_State *L) {
// }

// static int _uscript_func_util_shell_exec(lua_State *L) {
// }

static const struct luaL_Reg mle_lib[] = {
    { "buffer_add_mark", _uscript_func_buffer_add_mark },
    { "buffer_add_mark_ex", _uscript_func_buffer_add_mark_ex },
    { "buffer_add_srule", _uscript_func_buffer_add_srule },
    { "buffer_apply_styles", _uscript_func_buffer_apply_styles },
    { "buffer_clear", _uscript_func_buffer_clear },
    { "buffer_delete", _uscript_func_buffer_delete },
    { "buffer_delete_w_bline", _uscript_func_buffer_delete_w_bline },
    { "buffer_destroy", _uscript_func_buffer_destroy },
    { "buffer_destroy_mark", _uscript_func_buffer_destroy_mark },
    { "buffer_get", _uscript_func_buffer_get },
    { "buffer_get_bline", _uscript_func_buffer_get_bline },
    { "buffer_get_bline_col", _uscript_func_buffer_get_bline_col },
    { "buffer_get_bline_w_hint", _uscript_func_buffer_get_bline_w_hint },
    { "buffer_get_lettered_mark", _uscript_func_buffer_get_lettered_mark },
    { "buffer_get_offset", _uscript_func_buffer_get_offset },
    { "buffer_insert", _uscript_func_buffer_insert },
    { "buffer_insert_w_bline", _uscript_func_buffer_insert_w_bline },
    { "buffer_new", _uscript_func_buffer_new },
    { "buffer_new_open", _uscript_func_buffer_new_open },
    { "buffer_open", _uscript_func_buffer_open },
    { "buffer_redo", _uscript_func_buffer_redo },
    { "buffer_redo_action_group", _uscript_func_buffer_redo_action_group },
    { "buffer_register_append", _uscript_func_buffer_register_append },
    { "buffer_register_clear", _uscript_func_buffer_register_clear },
    { "buffer_register_get", _uscript_func_buffer_register_get },
    { "buffer_register_prepend", _uscript_func_buffer_register_prepend },
    { "buffer_register_set", _uscript_func_buffer_register_set },
    { "buffer_remove_srule", _uscript_func_buffer_remove_srule },
    { "buffer_replace", _uscript_func_buffer_replace },
    { "buffer_replace_w_bline", _uscript_func_buffer_replace_w_bline },
    { "buffer_save", _uscript_func_buffer_save },
    { "buffer_save_as", _uscript_func_buffer_save_as },
    { "buffer_set", _uscript_func_buffer_set },
    { "buffer_set_action_group_ptr", _uscript_func_buffer_set_action_group_ptr },
    { "buffer_set_callback", _uscript_func_buffer_set_callback },
    { "buffer_set_mmapped", _uscript_func_buffer_set_mmapped },
    { "buffer_set_styles_enabled", _uscript_func_buffer_set_styles_enabled },
    { "buffer_set_tab_width", _uscript_func_buffer_set_tab_width },
    { "buffer_substr", _uscript_func_buffer_substr },
    { "buffer_undo", _uscript_func_buffer_undo },
    { "buffer_undo_action_group", _uscript_func_buffer_undo_action_group },
    { "buffer_write_to_fd", _uscript_func_buffer_write_to_fd },
    { "buffer_write_to_file", _uscript_func_buffer_write_to_file },
    { "bview_add_cursor", _uscript_func_bview_add_cursor },
    { "bview_add_cursor_asleep", _uscript_func_bview_add_cursor_asleep },
    { "bview_center_viewport_y", _uscript_func_bview_center_viewport_y },
    { "bview_destroy", _uscript_func_bview_destroy },
    { "bview_draw", _uscript_func_bview_draw },
    { "bview_draw_cursor", _uscript_func_bview_draw_cursor },
    { "bview_get_active_cursor_count", _uscript_func_bview_get_active_cursor_count },
    { "bview_get_split_root", _uscript_func_bview_get_split_root },
    { "bview_max_viewport_y", _uscript_func_bview_max_viewport_y },
    { "bview_new", _uscript_func_bview_new },
    { "bview_open", _uscript_func_bview_open },
    { "bview_pop_kmap", _uscript_func_bview_pop_kmap },
    { "bview_push_kmap", _uscript_func_bview_push_kmap },
    { "bview_rectify_viewport", _uscript_func_bview_rectify_viewport },
    { "bview_remove_cursor", _uscript_func_bview_remove_cursor },
    { "bview_remove_cursors_except", _uscript_func_bview_remove_cursors_except },
    { "bview_resize", _uscript_func_bview_resize },
    { "bview_set_syntax", _uscript_func_bview_set_syntax },
    { "bview_set_viewport_y", _uscript_func_bview_set_viewport_y },
    { "bview_split", _uscript_func_bview_split },
    { "bview_wake_sleeping_cursors", _uscript_func_bview_wake_sleeping_cursors },
    { "bview_zero_viewport_y", _uscript_func_bview_zero_viewport_y },
    { "cursor_clone", _uscript_func_cursor_clone },
    { "cursor_cut_copy", _uscript_func_cursor_cut_copy },
    { "cursor_destroy", _uscript_func_cursor_destroy },
    { "cursor_drop_anchor", _uscript_func_cursor_drop_anchor },
    { "cursor_get_anchor", _uscript_func_cursor_get_anchor },
    { "cursor_get_lo_hi", _uscript_func_cursor_get_lo_hi },
    { "cursor_get_mark", _uscript_func_cursor_get_mark },
    { "cursor_lift_anchor", _uscript_func_cursor_lift_anchor },
    { "cursor_replace", _uscript_func_cursor_replace },
    { "cursor_select_between", _uscript_func_cursor_select_between },
    { "cursor_select_by", _uscript_func_cursor_select_by },
    { "cursor_select_by_bracket", _uscript_func_cursor_select_by_bracket },
    { "cursor_select_by_string", _uscript_func_cursor_select_by_string },
    { "cursor_select_by_word", _uscript_func_cursor_select_by_word },
    { "cursor_select_by_word_back", _uscript_func_cursor_select_by_word_back },
    { "cursor_select_by_word_forward", _uscript_func_cursor_select_by_word_forward },
    { "cursor_toggle_anchor", _uscript_func_cursor_toggle_anchor },
    { "cursor_uncut", _uscript_func_cursor_uncut },
    { "editor_bview_edit_count", _uscript_func_editor_bview_edit_count },
    { "editor_close_bview", _uscript_func_editor_close_bview },
    { "editor_count_bviews_by_buffer", _uscript_func_editor_count_bviews_by_buffer },
    { "editor_destroy_observer", _uscript_func_editor_destroy_observer },
    { "editor_display", _uscript_func_editor_display },
    { "editor_force_redraw", _uscript_func_editor_force_redraw },
    { "editor_get_input", _uscript_func_editor_get_input },
    { "editor_menu", _uscript_func_editor_menu },
    { "editor_notify_observers", _uscript_func_editor_notify_observers },
    { "editor_open_bview", _uscript_func_editor_open_bview },
    { "editor_prompt", _uscript_func_editor_prompt },
    { "editor_register_cmd", _uscript_func_editor_register_cmd },
    { "editor_register_observer", _uscript_func_editor_register_observer },
    { "editor_set_active", _uscript_func_editor_set_active },
    { "mark_clone", _uscript_func_mark_clone },
    { "mark_clone_w_letter", _uscript_func_mark_clone_w_letter },
    { "mark_delete_after", _uscript_func_mark_delete_after },
    { "mark_delete_before", _uscript_func_mark_delete_before },
    { "mark_delete_between_mark", _uscript_func_mark_delete_between_mark },
    { "mark_destroy", _uscript_func_mark_destroy },
    { "mark_find_bracket_pair", _uscript_func_mark_find_bracket_pair },
    { "mark_find_bracket_top", _uscript_func_mark_find_bracket_top },
    { "mark_find_next_re", _uscript_func_mark_find_next_re },
    { "mark_find_next_str", _uscript_func_mark_find_next_str },
    { "mark_find_prev_re", _uscript_func_mark_find_prev_re },
    { "mark_find_prev_str", _uscript_func_mark_find_prev_str },
    { "mark_get_between_mark", _uscript_func_mark_get_between_mark },
    { "mark_get_char_after", _uscript_func_mark_get_char_after },
    { "mark_get_char_before", _uscript_func_mark_get_char_before },
    { "mark_get_nchars_between", _uscript_func_mark_get_nchars_between },
    { "mark_get_offset", _uscript_func_mark_get_offset },
    { "mark_insert_after", _uscript_func_mark_insert_after },
    { "mark_insert_before", _uscript_func_mark_insert_before },
    { "mark_is_after_col_minus_lefties", _uscript_func_mark_is_after_col_minus_lefties },
    { "mark_is_at_bol", _uscript_func_mark_is_at_bol },
    { "mark_is_at_eol", _uscript_func_mark_is_at_eol },
    { "mark_is_at_word_bound", _uscript_func_mark_is_at_word_bound },
    { "mark_is_between", _uscript_func_mark_is_between },
    { "mark_is_eq", _uscript_func_mark_is_eq },
    { "mark_is_gt", _uscript_func_mark_is_gt },
    { "mark_is_gte", _uscript_func_mark_is_gte },
    { "mark_is_lt", _uscript_func_mark_is_lt },
    { "mark_is_lte", _uscript_func_mark_is_lte },
    { "mark_join", _uscript_func_mark_join },
    { "mark_move_beginning", _uscript_func_mark_move_beginning },
    { "mark_move_bol", _uscript_func_mark_move_bol },
    { "mark_move_bracket_pair", _uscript_func_mark_move_bracket_pair },
    { "mark_move_bracket_pair_ex", _uscript_func_mark_move_bracket_pair_ex },
    { "mark_move_bracket_top", _uscript_func_mark_move_bracket_top },
    { "mark_move_bracket_top_ex", _uscript_func_mark_move_bracket_top_ex },
    { "mark_move_by", _uscript_func_mark_move_by },
    { "mark_move_col", _uscript_func_mark_move_col },
    { "mark_move_end", _uscript_func_mark_move_end },
    { "mark_move_eol", _uscript_func_mark_move_eol },
    { "mark_move_next_re", _uscript_func_mark_move_next_re },
    { "mark_move_next_re_ex", _uscript_func_mark_move_next_re_ex },
    { "mark_move_next_re_nudge", _uscript_func_mark_move_next_re_nudge },
    { "mark_move_next_str", _uscript_func_mark_move_next_str },
    { "mark_move_next_str_ex", _uscript_func_mark_move_next_str_ex },
    { "mark_move_next_str_nudge", _uscript_func_mark_move_next_str_nudge },
    { "mark_move_offset", _uscript_func_mark_move_offset },
    { "mark_move_prev_re", _uscript_func_mark_move_prev_re },
    { "mark_move_prev_re_ex", _uscript_func_mark_move_prev_re_ex },
    { "mark_move_prev_str", _uscript_func_mark_move_prev_str },
    { "mark_move_prev_str_ex", _uscript_func_mark_move_prev_str_ex },
    { "mark_move_to", _uscript_func_mark_move_to },
    { "mark_move_to_w_bline", _uscript_func_mark_move_to_w_bline },
    { "mark_move_vert", _uscript_func_mark_move_vert },
    { "mark_replace", _uscript_func_mark_replace },
    { "mark_replace_between_mark", _uscript_func_mark_replace_between_mark },
    { "mark_swap_with_mark", _uscript_func_mark_swap_with_mark },
    { "util_escape_shell_arg", _uscript_func_util_escape_shell_arg },
    { "util_shell_exec", _uscript_func_util_shell_exec },
    { NULL, NULL }
};
