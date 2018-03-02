#include "mle.h"

static int _uscript_cmd(cmd_context_t* ctx);
static int _uscript_observer_cb(cmd_context_t* ctx);
static int _uscript_cmd_ex(cmd_context_t* ctx, int is_observer_cb);
static void _uscript_write(WrenVM* vm, const char* text);
static void _uscript_error(WrenVM* vm, WrenErrorType type, const char* module_name, int line_num, const char* message);
static void* wrenGetSlotPointer(WrenVM* vm, int slot);
static void wrenSetSlotPointer(WrenVM* vm, int slot, void* ptr);
static char* wrenGetSlotNullableString(WrenVM* vm, int slot);
static void wrenSetSlotNullableString(WrenVM* vm, int slot, char* str);
static int wrenInterpretFile(WrenVM* vm, char* path);
static WrenHandle* _uscript_make_cmd_map(uscript_t* uscript, cmd_context_t* ctx);
static void _uscript_editor_prompt(WrenVM* vm);
static void _uscript_editor_register_cmd(WrenVM* vm);
static void _uscript_editor_register_observer(WrenVM* vm);
static void _uscript_editor_get_input(WrenVM* vm);
static void _uscript_editor_menu(WrenVM* vm);
static void _uscript_bview_pop_kmap(WrenVM* vm);
static void _uscript_bview_push_kmap(WrenVM* vm);
#include "uscript.inc"

// Run uscript
uscript_t* uscript_run(editor_t* editor, char* path) {
    uscript_t* uscript;
    WrenConfiguration config;

    uscript = calloc(1, sizeof(uscript_t));
    uscript->editor = editor;

    wrenInitConfiguration(&config);
    config.bindForeignMethodFn = _uscript_bind_method;
    config.writeFn = _uscript_write;
    config.errorFn = _uscript_error;
    config.userData = (void*)uscript;

    uscript->vm = wrenNewVM(&config);
    if (wrenInterpret(uscript->vm, mle_wren) != WREN_RESULT_SUCCESS
        || wrenInterpretFile(uscript->vm, path) != MLE_OK
    ) {
        uscript_destroy(uscript);
        return NULL;
    }

    wrenEnsureSlots(uscript->vm, 1);
    wrenGetVariable(uscript->vm, "main", "mle", 0);
    uscript->class_mle = wrenGetSlotHandle(uscript->vm, 0);
    uscript->func_list2map = wrenMakeCallHandle(uscript->vm, "list2map(_,_)");

    return uscript;
}

// Destroy uscript
int uscript_destroy(uscript_t* uscript) {
    uhandle_t* uhandle;
    uhandle_t* uhandle_tmp;
    DL_FOREACH_SAFE(uscript->uhandles, uhandle, uhandle_tmp) {
        DL_DELETE(uscript->uhandles, uhandle);
        wrenReleaseHandle(uscript->vm, uhandle->receiver);
        wrenReleaseHandle(uscript->vm, uhandle->method);
        free(uhandle);
    }
    if (uscript->class_mle) wrenReleaseHandle(uscript->vm, uscript->class_mle);
    if (uscript->func_list2map) wrenReleaseHandle(uscript->vm, uscript->func_list2map);
    wrenFreeVM(uscript->vm);
    free(uscript);
    return MLE_OK;
}

static int _uscript_cmd(cmd_context_t* ctx) {
    return _uscript_cmd_ex(ctx, 0);
}

static int _uscript_observer_cb(cmd_context_t* ctx) {
    return _uscript_cmd_ex(ctx, 1);
}

// Invoke cmd in uscript
static int _uscript_cmd_ex(cmd_context_t* ctx, int is_observer_cb) {
    WrenVM* vm;
    WrenHandle* cmd_map;
    uhandle_t* uhandle;
    uhandle = (uhandle_t*)(is_observer_cb ? ctx->observer_udata : ctx->cmd->udata);
    vm = uhandle->uscript->vm;
    if (!(cmd_map = _uscript_make_cmd_map(uhandle->uscript, ctx))) {
        return MLE_ERR;
    }
    wrenEnsureSlots(vm, 2);
    wrenSetSlotHandle(vm, 0, uhandle->receiver);
    wrenSetSlotHandle(vm, 1, cmd_map);
    wrenReleaseHandle(vm, cmd_map);
    if (wrenCall(vm, uhandle->method) != WREN_RESULT_SUCCESS) {
        return MLE_ERR;
    } else if (wrenGetSlotType(vm, 0) == WREN_TYPE_BOOL && !wrenGetSlotBool(vm, 0)) {
        return MLE_ERR;
    }
    return MLE_OK;
}

// Handle write from uscript
static void _uscript_write(WrenVM* vm, const char* text) {
    uscript_t* uscript;
    uscript = (uscript_t*)wrenGetUserData(vm);
    if (!uscript->editor->active_edit) return;
    mark_insert_before(uscript->editor->active_edit->active_cursor->mark, (char*)text, strlen(text));
}

// Handle error from uscript
static void _uscript_error(WrenVM* vm, WrenErrorType type, const char* module_name, int line_num, const char* message) {
    MLE_LOG_ERR("Error in uscript %s:%d: %s\n", module_name, line_num, message);
}

// Get pointer from Wren slot as string
static void* wrenGetSlotPointer(WrenVM* vm, int slot) {
    const char* ptr;
    if (wrenGetSlotType(vm, slot) == WREN_TYPE_STRING) {
        ptr = wrenGetSlotString(vm, slot);
        if (strlen(ptr) > 0) {
            return (void*)strtoull(ptr, NULL, 16);
        }
    }
    return NULL;
}

// Set pointer to Wren slot as string
static void wrenSetSlotPointer(WrenVM* vm, int slot, void* ptr) {
    char ptrbuf[32];
    if (ptr == NULL) {
        wrenSetSlotNull(vm, slot);
    } else {
        snprintf(ptrbuf, 32, "%llx", (unsigned long long)ptr);
        wrenSetSlotString(vm, slot, ptrbuf);
    }
}

// Get nullable string from Wren slot
static char* wrenGetSlotNullableString(WrenVM* vm, int slot) {
    if (wrenGetSlotType(vm, slot) == WREN_TYPE_STRING) {
        return (char*)wrenGetSlotString(vm, slot);
    }
    return NULL;
}

// Set nullable string to Wren slot
static void wrenSetSlotNullableString(WrenVM* vm, int slot, char* str) {
    if (str == NULL) {
        wrenSetSlotNull(vm, slot);
    } else {
        wrenSetSlotString(vm, slot, (const char*)str);
    }
}

// Interpret path in Wren VM
static int wrenInterpretFile(WrenVM* vm, char* path) {
    char* code;
    FILE* fp;
    long size;
    int rv;

    fp = fopen(path, "rb");
    if (!fp) return MLE_ERR;

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    rv = MLE_ERR;
    code = malloc(size+1);
    if (code && fread(code, 1, size, fp) == size) {
        code[size] = '\0';
        if (wrenInterpret(vm, code) == WREN_RESULT_SUCCESS) {
            rv = MLE_OK;
        }
    }
    if (code) free(code);
    fclose(fp);

    return rv;
}

static WrenHandle* _uscript_make_cmd_map(uscript_t* uscript, cmd_context_t* ctx) {
    // TODO Refactor when wrenSetSlotNewMap is available
    WrenVM* vm;
    char input_str[16];
    vm = uscript->vm;
    editor_input_to_key(ctx->editor, &ctx->input, input_str);
    wrenEnsureSlots(vm, 4); // 0 (mle_class), 1 (keys), 2 (vals), 3 (tmp)
    wrenSetSlotHandle(vm, 0, uscript->class_mle);
    wrenSetSlotNewList(vm, 1);
    wrenSetSlotString(vm, 3, "editor");       wrenInsertInList(vm, 1, -1, 3);
    wrenSetSlotString(vm, 3, "loop_ctx");     wrenInsertInList(vm, 1, -1, 3);
    wrenSetSlotString(vm, 3, "cmd");          wrenInsertInList(vm, 1, -1, 3);
    wrenSetSlotString(vm, 3, "buffer");       wrenInsertInList(vm, 1, -1, 3);
    wrenSetSlotString(vm, 3, "bview");        wrenInsertInList(vm, 1, -1, 3);
    wrenSetSlotString(vm, 3, "cursor");       wrenInsertInList(vm, 1, -1, 3);
    wrenSetSlotString(vm, 3, "mark");         wrenInsertInList(vm, 1, -1, 3);
    wrenSetSlotString(vm, 3, "input");        wrenInsertInList(vm, 1, -1, 3);
    wrenSetSlotString(vm, 3, "static_param"); wrenInsertInList(vm, 1, -1, 3);
    wrenSetSlotNewList(vm, 2);
    wrenSetSlotPointer(vm, 3, (void*)ctx->editor);       wrenInsertInList(vm, 2, -1, 3);
    wrenSetSlotPointer(vm, 3, (void*)ctx->loop_ctx);     wrenInsertInList(vm, 2, -1, 3);
    wrenSetSlotPointer(vm, 3, (void*)ctx->cmd);          wrenInsertInList(vm, 2, -1, 3);
    wrenSetSlotPointer(vm, 3, (void*)ctx->buffer);       wrenInsertInList(vm, 2, -1, 3);
    wrenSetSlotPointer(vm, 3, (void*)ctx->bview);        wrenInsertInList(vm, 2, -1, 3);
    wrenSetSlotPointer(vm, 3, (void*)ctx->cursor);       wrenInsertInList(vm, 2, -1, 3);
    wrenSetSlotPointer(vm, 3, (void*)ctx->cursor->mark); wrenInsertInList(vm, 2, -1, 3);
    wrenSetSlotString(vm, 3, (const char*)input_str);    wrenInsertInList(vm, 2, -1, 3);
    wrenSetSlotNullableString(vm, 3, ctx->static_param); wrenInsertInList(vm, 2, -1, 3);
    if (wrenCall(vm, uscript->func_list2map) != WREN_RESULT_SUCCESS) {
        return NULL;
    }
    return wrenGetSlotHandle(vm, 0);
}

// foreign static editor_prompt(editor, prompt)
static void _uscript_editor_prompt(WrenVM* vm) {
    int rv;
    editor_t* editor;
    char* prompt;
    editor_prompt_params_t* params;
    char* optret_answer = NULL;
    editor = (editor_t*)wrenGetSlotPointer(vm, 1);
    prompt = (char*)wrenGetSlotNullableString(vm, 2);
    params = NULL;
    rv = editor_prompt(editor, prompt, params, &optret_answer);
    wrenEnsureSlots(vm, 3);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, (double)rv);
    wrenInsertInList(vm, 0, -1, 1);
    wrenSetSlotNullableString(vm, 2, (char*)optret_answer);
    wrenInsertInList(vm, 0, -1, 2);
    free(optret_answer);
}

// foreign static editor_register_cmd(cmd_name, cmd_fn)
static void _uscript_editor_register_cmd(WrenVM* vm) {
    uscript_t* uscript;
    uhandle_t* uhandle;
    int rv;
    cmd_t cmd = {0};
    uscript = (uscript_t*)wrenGetUserData(vm);
    uhandle = calloc(1, sizeof(uhandle_t));
    uhandle->uscript = uscript;
    uhandle->receiver = wrenGetSlotHandle(vm, 2);
    uhandle->method = wrenMakeCallHandle(vm, "call(_)");
    DL_APPEND(uscript->uhandles, uhandle);
    cmd.name = (char*)wrenGetSlotString(vm, 1); // strdup'd by editor_register_cmd
    cmd.func = _uscript_cmd;
    cmd.udata = (void*)uhandle;
    rv = editor_register_cmd(uscript->editor, &cmd);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, rv == MLE_OK ? 1 : 0);
    wrenInsertInList(vm, 0, -1, 1);
}

// foreign static editor_register_observer(cmd_name, is_before, notify_fn)
static void _uscript_editor_register_observer(WrenVM* vm) {
    uscript_t* uscript;
    uhandle_t* uhandle;
    int rv;
    char* cmd_name;
    int is_before;
    uscript = (uscript_t*)wrenGetUserData(vm);
    uhandle = calloc(1, sizeof(uhandle_t));
    uhandle->uscript = uscript;
    uhandle->receiver = wrenGetSlotHandle(vm, 3);
    uhandle->method = wrenMakeCallHandle(vm, "call(_)");
    DL_APPEND(uscript->uhandles, uhandle);
    cmd_name = (char*)wrenGetSlotString(vm, 1); // strdup'd by editor_register_observer
    is_before = (int)wrenGetSlotDouble(vm, 2);
    rv = editor_register_observer(uscript->editor, cmd_name, (void*)uhandle, is_before, _uscript_observer_cb, NULL);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);
    wrenSetSlotDouble(vm, 1, rv == MLE_OK ? 1 : 0);
    wrenInsertInList(vm, 0, -1, 1);
}

// foreign static editor_get_input(editor)
static void _uscript_editor_get_input(WrenVM* vm) {
    // TODO
}

// foreign static editor_menu(editor)
static void _uscript_editor_menu(WrenVM* vm) {
    // TODO
}

// foreign static bview_pop_kmap(editor, kmap_name)
static void _uscript_bview_pop_kmap(WrenVM* vm) {
    // TODO
}

// foreign static bview_push_kmap(editor, kmap)
static void _uscript_bview_push_kmap(WrenVM* vm) {
    // TODO
}
