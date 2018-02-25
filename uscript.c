#include "mle.h"

static int _uscript_cmd(cmd_context_t* ctx);
static WrenForeignMethodFn _uscript_bind_method(WrenVM* vm, const char* module_name, const char* class_name, bool is_static, const char* sig);
static void _uscript_write(WrenVM* vm, const char* text);
static void _uscript_error(WrenVM* vm, WrenErrorType type, const char* module_name, int line_num, const char* message);
static void* wrenGetSlotPointer(WrenVM* vm, int slot);
static void wrenSetSlotPointer(WrenVM* vm, int slot, void* ptr);
static int wrenInterpretFile(WrenVM* vm, char* path);
static void _uscript_bview_pop_kmap(WrenVM* vm);
static void _uscript_bview_push_kmap(WrenVM* vm);
static void _uscript_editor_get_input(WrenVM* vm);
static void _uscript_editor_menu(WrenVM* vm);
static void _uscript_editor_prompt(WrenVM* vm);
static void _uscript_editor_register_cmd(WrenVM* vm);

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
        free(uscript);
        return NULL;
    }

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
    wrenFreeVM(uscript->vm);
    free(uscript);
    return MLE_OK;
}

// Invoke cmd in uscript
static int _uscript_cmd(cmd_context_t* ctx) {
    WrenVM* vm;
    uhandle_t* uhandle;
    uhandle = (uhandle_t*)ctx->cmd->udata;
    vm = uhandle->uscript->vm;
    wrenEnsureSlots(vm, 2);
    wrenSetSlotHandle(vm, 0, uhandle->receiver);
    wrenSetSlotPointer(vm, 1, ctx->cursor->mark); // TODO context as map
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
    _uscript_write(vm, message); // TODO something else?
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

static void _uscript_bview_pop_kmap(WrenVM* vm) {
    // TODO
}

static void _uscript_bview_push_kmap(WrenVM* vm) {
    // TODO
}

static void _uscript_editor_get_input(WrenVM* vm) {
    // TODO
}

static void _uscript_editor_menu(WrenVM* vm) {
    // TODO
}

static void _uscript_editor_prompt(WrenVM* vm) {
    // TODO
}

static void _uscript_editor_register_cmd(WrenVM* vm) {
    uscript_t* uscript;
    cmd_t cmd = {0};
    uscript = (uscript_t*)wrenGetUserData(vm);
    uhandle_t* uhandle;
    int rv;
    uhandle = calloc(1, sizeof(uhandle_t));
    uhandle->uscript = uscript;
    uhandle->receiver = wrenGetSlotHandle(vm, 2);
    uhandle->method = wrenMakeCallHandle(vm, "call(_)");
    DL_APPEND(uscript->uhandles, uhandle);
    cmd.name = (char*)wrenGetSlotString(vm, 1); // strdup'd by editor_register_cmd
    cmd.func = _uscript_cmd;
    cmd.udata = (void*)uhandle;
    rv = editor_register_cmd(uscript->editor, &cmd);
    wrenSetSlotBool(vm, 0, rv == MLE_OK ? 1 : 0);
}
