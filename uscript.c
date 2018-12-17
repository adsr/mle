#include "mle.h"

#define MLE_USCRIPT_KEY "_uscript"

#define MLE_USCRIPT_GET(pl, pu) do { \
    lua_getglobal((pl), MLE_USCRIPT_KEY); \
    (pu) = luaL_optpointer((pl), -1, NULL); \
    if (!(pu)) return 0; \
} while(0)

#define luaL_pushkey(pL, pt, pk, pv) do { \
    lua_pushstring((pL), (pk)); \
    (lua_push ## pt) ((pL), (pv)); \
    lua_settable((pL), -3); \
} while (0)

#define luaL_pushkey2(pL, pt, pk, pv1, pv2) do { \
    lua_pushstring((pL), (pk)); \
    (lua_push ## pt) ((pL), (pv1), (pv2)); \
    lua_settable((pL), -3); \
} while (0)

static int _uscript_panic(lua_State* L);
static int _uscript_cmd_cb(cmd_context_t* ctx);
static int _uscript_observer_cb(char* event_name, void* event_data, void* udata);
static int _uscript_write(lua_State* L);
static void _uscript_push_event_map(uscript_t* uscript, char* event_name, void* event_data);
static void _uscript_push_cmd_map(lua_State* L, cmd_context_t* ctx);
static void _uscript_push_baction_map(lua_State* L, baction_t* baction);
static int _uscript_func_editor_prompt(lua_State* L);
static int _uscript_func_editor_register_cmd(lua_State* L);
static int _uscript_func_editor_register_observer(lua_State* L);
static int _uscript_func_util_escape_shell_arg(lua_State* L);
static int _uscript_func_util_shell_exec(lua_State* L);
static int _uscript_func_editor_get_input(lua_State* L);
static int _uscript_func_editor_menu(lua_State* L);
static int _uscript_func_bview_pop_kmap(lua_State* L);
static int _uscript_func_bview_push_kmap(lua_State* L);
static int _uscript_func_buffer_set_callback(lua_State* L);
static int _uscript_func_buffer_add_srule(lua_State* L);
static int _uscript_func_buffer_remove_srule(lua_State* L);
static int _uscript_func_buffer_write_to_file(lua_State* L);
static int _uscript_func_editor_input_to_key(lua_State* L);
static void* luaL_checkpointer(lua_State* L, int arg);
static void* luaL_optpointer(lua_State* L, int arg, void* def);
static void lua_pushpointer(lua_State* L, void* ptr);
static int luaL_checkfunction(lua_State* L, int arg);
static int luaopen_mle(lua_State *L);

#include "uscript.inc"

// Run uscript
uscript_t* uscript_run(editor_t* editor, char* path) {
    lua_State* L;
    uscript_t* uscript;
    L = luaL_newstate();
    luaL_openlibs(L);
    luaL_requiref(L, "mle", luaopen_mle, 1);

    uscript = calloc(1, sizeof(uscript_t));
    uscript->editor = editor;
    uscript->L = L;

    lua_pushpointer(L, (void*)uscript);
    lua_setglobal(L, MLE_USCRIPT_KEY);
    lua_pop(L, 1);

    lua_getglobal(L, "_G");
    lua_pushcfunction(L, _uscript_write);
    lua_setfield(L, -2, "print");
    lua_pop(L, 1);

    lua_atpanic(L, _uscript_panic);


    luaL_loadfile(L, path); // TODO err

    lua_pcall(L, 0, 0, 0);
    return uscript;
}

// Destroy uscript
int uscript_destroy(uscript_t* uscript) {
    lua_close(uscript->L);
    return MLE_OK;
}

static int _uscript_panic(lua_State* L) {
   MLE_SET_ERR(&_editor, "uscript panic: %s", lua_tostring(L, -1));
   return 0;
}

// Invoke cmd in uscript
static int _uscript_cmd_cb(cmd_context_t* ctx) {
    int rv;
    lua_State* L;
    uhandle_t* uhandle;
    int top;

    uhandle = (uhandle_t*)(ctx->cmd->udata);
    L = uhandle->uscript->L;
    top = lua_gettop(L);
    lua_rawgeti(L, LUA_REGISTRYINDEX, uhandle->callback_ref);
    _uscript_push_cmd_map(L, ctx);

    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
        printf("err[%s]\n", luaL_checkstring(L, -1));
        rv = MLE_ERR;
    } else if (lua_isboolean(L, -1) && !lua_toboolean(L, -1)) {
        rv = MLE_ERR;
    } else {
        rv = MLE_OK;
    }

    lua_settop(L, top);
    return rv;
}

static int _uscript_observer_cb(char* event_name, void* event_data, void* udata) {
    int rv;
    lua_State* L;
    uhandle_t* uhandle;
    uhandle = (uhandle_t*)(udata);
    L = uhandle->uscript->L;

    lua_rawgeti(L, LUA_REGISTRYINDEX, uhandle->callback_ref);
    _uscript_push_event_map(uhandle->uscript, event_name, event_data);

    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
        printf("err[%s]\n", luaL_checkstring(L, -1));
        rv = MLE_ERR;
    } else if (lua_isboolean(L, -1) && !lua_toboolean(L, -1)) {
        rv = MLE_ERR;
    } else {
        rv = MLE_OK;
    }

    return rv;
}

// Handle write from uscript
static int _uscript_write(lua_State* L) {
    uscript_t* uscript;
    char* str;
    mark_t* mark;
    int i, nargs;
    nargs = lua_gettop(L);
    MLE_USCRIPT_GET(L, uscript);
    if (!uscript->editor->active_edit
        || !uscript->editor->active_edit->active_cursor
    ) {
        return 0;
    }
    mark = uscript->editor->active_edit->active_cursor->mark;
    for (i = 1; i <= nargs; i++) {
        str = (char*)lua_tostring(L, i);
        mark_insert_before(mark, str, strlen(str));
    }
    return 0;
}

static void _uscript_push_event_map(uscript_t* uscript, char* event_name, void* event_data) {
    lua_State* L;
    L = uscript->L;
    if (strcmp(event_name, "buffer:baction") == 0) {
        _uscript_push_baction_map(L, (baction_t*)event_data);
        return;
    } else if (strcmp(event_name, "buffer:save") == 0) {
        lua_createtable(L, 0, 1);
        luaL_pushkey(L, pointer, "bview", event_data);
        return;
    } else if (strncmp(event_name, "cmd:", 4) == 0) {
        _uscript_push_cmd_map(L, (cmd_context_t*)event_data);
        return;
    }
    lua_pushnil(uscript->L); // TODO
}

static void _uscript_push_cmd_map(lua_State* L, cmd_context_t* ctx) {
    char input_str[16];
    editor_input_to_key(ctx->editor, &ctx->input, input_str);
    lua_createtable(L, 0, 1);
    luaL_pushkey(L, pointer, "editor",       (void*)ctx->editor);
    luaL_pushkey(L, pointer, "loop_ctx",     (void*)ctx->loop_ctx);
    luaL_pushkey(L, pointer, "cmd",          (void*)ctx->cmd);
    luaL_pushkey(L, pointer, "buffer",       (void*)ctx->buffer);
    luaL_pushkey(L, pointer, "bview",        (void*)ctx->bview);
    luaL_pushkey(L, pointer, "cursor",       (void*)ctx->cursor);
    luaL_pushkey(L, pointer, "mark",         (void*)ctx->cursor->mark);
    luaL_pushkey(L, string,  "static_param", (const char*)ctx->static_param);
    luaL_pushkey(L, string,  "input",        (const char*)input_str);
}

static void _uscript_push_baction_map(lua_State* L, baction_t* baction) {
    lua_createtable(L, 0, 1);
    luaL_pushkey(L,  integer, "type",                 baction->type);
    luaL_pushkey(L,  pointer, "buffer",               (void*)baction->buffer);
    luaL_pushkey(L,  integer, "start_line_index",     baction->start_line_index);
    luaL_pushkey(L,  integer, "start_col",            baction->start_col);
    luaL_pushkey(L,  integer, "maybe_end_line_index", baction->maybe_end_line_index);
    luaL_pushkey(L,  integer, "maybe_end_col",        baction->maybe_end_col);
    luaL_pushkey(L,  integer, "byte_delta",           baction->byte_delta);
    luaL_pushkey(L,  integer, "char_delta",           baction->char_delta);
    luaL_pushkey(L,  integer, "line_delta",           baction->line_delta);
    luaL_pushkey2(L, lstring, "data",                 (const char*)baction->data, baction->data_len);
}

// foreign static string _uscript_func_editor_prompt(prompt)
static int _uscript_func_editor_prompt(lua_State* L) {
    uscript_t* uscript;
    char* prompt;
    char* answer = NULL;
    MLE_USCRIPT_GET(L, uscript);
    prompt = (char*)luaL_checkstring(L, 1);
    if (editor_prompt(uscript->editor, prompt, NULL, &answer) == MLE_OK && answer) {
        lua_pushstring(L, answer);
        free(answer);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

int editor_register_cmd(editor_t* editor, cmd_t* cmd);

// foreign static int _uscript_func_editor_register_cmd(cmd_name, fn_callback)
static int _uscript_func_editor_register_cmd(lua_State* L) {
    uscript_t* uscript;
    uhandle_t* uhandle;
    int rv;
    char* cmd_name;
    int fn_callback;
    cmd_t cmd = {0};
    MLE_USCRIPT_GET(L, uscript);

    cmd_name = (char*)luaL_checkstring(L, 1); // strdup'd by editor_register_cmd
    fn_callback = luaL_checkfunction(L, 2);

    uhandle = calloc(1, sizeof(uhandle_t));
    uhandle->uscript = uscript;
    uhandle->callback_ref = fn_callback;
    DL_APPEND(uscript->uhandles, uhandle);

    cmd.name = cmd_name;
    cmd.func = _uscript_cmd_cb;
    cmd.udata = (void*)uhandle;
    rv = editor_register_cmd(uscript->editor, &cmd);

    lua_createtable(L, 0, 1);
    luaL_pushkey(L, integer, "rv", rv);
    lua_pushvalue(L, -1);
    return 1;
}

// foreign static int _uscript_func_editor_register_observer(event_name, fn_callback)
static int _uscript_func_editor_register_observer(lua_State* L) {
    int rv;
    char* event_name;
    int fn_callback;
    uscript_t* uscript;
    uhandle_t* uhandle;
    MLE_USCRIPT_GET(L, uscript);

    event_name = (char*)luaL_checkstring(L, 1);
    fn_callback = luaL_checkfunction(L, 2);

    uhandle = calloc(1, sizeof(uhandle_t));
    uhandle->uscript = uscript;
    uhandle->callback_ref = fn_callback;
    DL_APPEND(uscript->uhandles, uhandle);

    rv = editor_register_observer(uscript->editor, event_name, (void*)uhandle, _uscript_observer_cb, NULL);

    lua_createtable(L, 0, 1);
    luaL_pushkey(L, integer, "rv", rv);
    lua_pushvalue(L, -1);
    return 1;
}

// foreign static int _uscript_func_util_escape_shell_arg(arg)
static int _uscript_func_util_escape_shell_arg(lua_State* L) {
    (void)L;
    return 0;
}

// foreign static int _uscript_func_util_shell_exec(cmd, timeout_s)
static int _uscript_func_util_shell_exec(lua_State* L) {
    int rv;
    char* cmd;
    long timeout_s;
    uscript_t* uscript;
    char* output;
    size_t output_len;
    MLE_USCRIPT_GET(L, uscript);

    cmd = (char*)luaL_checkstring(L, 1);
    timeout_s = (long)luaL_checkinteger(L, 2);
    output = NULL;
    output_len = 0;
    rv = util_shell_exec(uscript->editor, cmd, timeout_s, NULL, 0, 0, NULL, &output, &output_len);

    lua_createtable(L, 0, 1);
    luaL_pushkey(L, integer, "rv", rv);
    luaL_pushkey2(L, lstring, "output", (output ? output : ""), output_len);
    lua_pushvalue(L, -1);
    if (output) free(output);
    return 1;
}

// foreign static int _uscript_func_editor_get_input(x, y, z)
static int _uscript_func_editor_get_input(lua_State* L) {
    // TODO
    (void)L;
    return 0;
}

// foreign static int _uscript_func_editor_menu(x, y, z)
static int _uscript_func_editor_menu(lua_State* L) {
    // TODO
    (void)L;
    return 0;
}

// foreign static int _uscript_func_bview_pop_kmap(x, y, z)
static int _uscript_func_bview_pop_kmap(lua_State* L) {
    // TODO
    (void)L;
    return 0;
}

// foreign static int _uscript_func_bview_push_kmap(x, y, z)
static int _uscript_func_bview_push_kmap(lua_State* L) {
    // TODO
    (void)L;
    return 0;
}

// foreign static int _uscript_func_buffer_set_callback(x, y, z)
static int _uscript_func_buffer_set_callback(lua_State* L) {
    // TODO
    (void)L;
    return 0;
}

// foreign static int _uscript_func_buffer_add_srule(x, y, z)
static int _uscript_func_buffer_add_srule(lua_State* L) {
    // TODO
    (void)L;
    return 0;
}

// foreign static int _uscript_func_buffer_remove_srule(x, y, z)
static int _uscript_func_buffer_remove_srule(lua_State* L) {
    // TODO
    (void)L;
    return 0;
}

// foreign static int _uscript_func_buffer_write_to_file(x, y, z)
static int _uscript_func_buffer_write_to_file(lua_State* L) {
    // TODO
    (void)L;
    return 0;
}

// foreign static int _uscript_func_editor_input_to_key(x, y, z)
static int _uscript_func_editor_input_to_key(lua_State* L) {
    // TODO
    (void)L;
    return 0;
}

static void* luaL_checkpointer(lua_State* L, int arg) {
    luaL_checktype(L, arg, LUA_TSTRING);
    return luaL_optpointer(L, arg, NULL);
}

static void* luaL_optpointer(lua_State* L, int arg, void* def) {
    const char* ptr;
    ptr = luaL_optstring(L, arg, NULL);
    if (ptr && strlen(ptr) > 0) {
        return (void*)strtoull(ptr, NULL, 16);
    }
    return def;
}

static void lua_pushpointer(lua_State* L, void* ptr) {
    char ptrbuf[32];
    if (ptr == NULL) {
        lua_pushnil(L);
    } else {
        snprintf(ptrbuf, 32, "%llx", (unsigned long long)ptr);
        lua_pushstring(L, ptrbuf);
    }
}

static int luaL_checkfunction(lua_State* L, int arg) {
    luaL_checktype(L, arg, LUA_TFUNCTION);
    lua_pushvalue(L, arg);
    return luaL_ref(L, LUA_REGISTRYINDEX);
}

static int luaopen_mle(lua_State *L) {
    luaL_newlib(L, mle_lib);
    return 1;
}
