#include <ctype.h>
#include "mle.h"

// http://www.glump.net/files/2012/08/vi-vim-cheat-sheet-and-tutorial.pdf
// http://jvi.sourceforge.net/vimhelp/index.txt.html

typedef struct {
    editor_t* editor;
    char* registers[128];
} vim_state_t;

typedef struct {
    char oper[3];
    char oper_direct[3];
    char motion[3];
    char reg;
    uintmax_t oper_num;
    uintmax_t motion_num;
} vim_command_t;

static int _cmd_vim_normal_eval(cmd_context_t* ctx, vim_state_t* vs);
static int _cmd_vim_normal_get_input(editor_t* editor, kinput_t* ret_input);
static int _cmd_vim_normal_get_input_til_enter(editor_t* editor, kinput_t** ret_inputs, int* ret_inputs_len);
static int _cmd_vim_normal_can_execute(vim_command_t* vimcmd);
static int _cmd_vim_normal_execute(cmd_context_t* ctx, vim_state_t* vs, vim_command_t* vimcmd);
static int _cmd_vim_normal_execute_motion(cmd_context_t* ctx, vim_state_t* vs, vim_command_t* vimcmd);
static int _cmd_vim_normal_execute_oper(cmd_context_t* ctx, vim_state_t* vs, vim_command_t* vimcmd);

int cmd_vim_normal(cmd_context_t* ctx) {
    int rc;
    if ((rc = _cmd_vim_normal_eval(ctx, (vim_state_t*)(*ctx->udata))) == MLE_ERR) {
        MLE_RETURN_ERR(ctx->editor, "%s", "Could not parse input string");
    }
    return MLE_OK;
}

int cmdinit_vim_normal(editor_t* editor, cmd_funcref_t* self, int is_deinit) {
    vim_state_t* vs;
    int i;
    if (!is_deinit) {
        vs = calloc(1, sizeof(vim_state_t));
        *((vim_state_t**)self->udata) = vs;
    } else {
        vs = *((vim_state_t**)self->udata);
        for (i = 0; i < 128; i++) {
            if (vs->registers[i]) free(vs->registers[i]);
        }
        free(vs);
        self->udata = NULL;
    }
    return MLE_OK;
}

static int _cmd_vim_normal_eval(cmd_context_t* ctx, vim_state_t* vs) {
    editor_t* editor;
    kinput_t input;
    kinput_t input_tmp;
    vim_command_t vimcmd;
    uintmax_t num_tmp;
    int has_next_input;
    uint32_t ch;
    int rc;

    editor = ctx->editor;
    memset(&input, 0, sizeof(kinput_t));
    memset(&input_tmp, 0, sizeof(kinput_t));
    memset(&vimcmd, 0, sizeof(vim_command_t));
    input = ctx->input;
    num_tmp = 0;
    has_next_input = 1;

    do {
        if (!has_next_input) {
            _cmd_vim_normal_get_input(editor, &input);
        } else {
            has_next_input = 0;
        }
        ch = (char)input.ch;
        if (ch == '"') {
            // Ingest register
            _cmd_vim_normal_get_input(editor, &input_tmp);
            vimcmd.reg = input_tmp.ch >= 0 && input_tmp.ch <= 128 ? input_tmp.ch : 0;
        } else if (isdigit(ch)) {
            // Ingest number
            num_tmp = ch - '0';
            while (1) {
                _cmd_vim_normal_get_input(editor, &input_tmp);
                if (isdigit(input_tmp.ch)) {
                    num_tmp *= 10;
                    num_tmp += ch - '0';
                } else {
                    break;
                }
            }
            if      (!vimcmd.oper_num)   vimcmd.oper_num = num_tmp;
            else if (!vimcmd.motion_num) vimcmd.motion_num = num_tmp;
            input = input_tmp;
            has_next_input = 1;
        } else if (strchr("`tT[]fF'", ch) != NULL) {
            _cmd_vim_normal_get_input(editor, &input_tmp);
            vimcmd.motion[0] = ch;
            vimcmd.motion[1] = input_tmp.ch; // TODO unicode
        } else if (strchr("/?#$%^*()_+-WwEe{}GHhjkLl;|BbNnM,", ch) != NULL) {
            vimcmd.motion[0] = ch;
        } else if (strchr("!=ydc<>", ch) != NULL) {
            vimcmd.oper[0] = ch;
            _cmd_vim_normal_get_input(editor, &input);
            if (ch == (char)input.ch) {
                vimcmd.oper[0] = input.ch;
            } else {
                has_next_input = 1;
            }
        } else if (strchr("@qrm", ch) != NULL) {
            _cmd_vim_normal_get_input(editor, &input);
            vimcmd.oper[0] = ch;
            vimcmd.oper[1] = input.ch;
        } else if (strchr("~YuIiOoPpAaSsDJxCVv.", ch) != NULL) {
            vimcmd.oper_direct[0] = ch;
        } else if (strchr("g&QRUK:", ch)) {
            return MLE_ERR; // TODO implement
        }
    } while ((rc = _cmd_vim_normal_can_execute(&vimcmd)) == 0);

    if (rc != 1) return MLE_ERR;

    return _cmd_vim_normal_execute(ctx, vs, &vimcmd);
}

static int _cmd_vim_normal_can_execute(vim_command_t* vimcmd) {
    return vimcmd->motion || vimcmd->oper_direct ? 1 : 0;
}

static int _cmd_vim_normal_execute(cmd_context_t* ctx, vim_state_t* vs, vim_command_t* vimcmd) {
    _cmd_vim_normal_execute_motion(ctx, vs, vimcmd);
    _cmd_vim_normal_execute_oper(ctx, vs, vimcmd);
    return MLE_OK;
}

static int _cmd_vim_normal_execute_motion(cmd_context_t* ctx, vim_state_t* vs, vim_command_t* vimcmd) {
    // "`tT[]fF'"
    // "/?#$%^*()_+-WwEe{}GHhjkLl;|BbNnM,"
    char* motion;
    motion = vimcmd->motion;
    return MLE_OK;
}

static int _cmd_vim_normal_execute_oper(cmd_context_t* ctx, vim_state_t* vs, vim_command_t* vimcmd) {
    // "!=ydc<>"
    // "@qrm"
    // "~YuIiOoPpAaSsDJxCVv."
    char* oper;
    oper = strlen(vimcmd->oper) > 0 ? vimcmd->oper : vimcmd->oper_direct;
    return MLE_OK;
}

static int _cmd_vim_normal_get_input(editor_t* editor, kinput_t* ret_input) {
    cmd_context_t ctx;
    memset(&ctx, 0, sizeof(cmd_context_t));
    editor_get_input(editor, &ctx);
    editor_display(editor);
    *ret_input = ctx.input;
    return MLE_OK;
}

static int _cmd_vim_normal_get_input_til_enter(editor_t* editor, kinput_t** ret_inputs, int* ret_inputs_len) {
    int len;
    int size;
    kinput_t* inputs;
    kinput_t input;

    len = 0;
    size = 0;
    inputs = NULL;
    memset(&input, 0, sizeof(kinput_t));

    while (1) {
        if (len >= size) {
            size += 32;
            inputs = realloc(inputs, size * sizeof(kinput_t));
        }
        _cmd_vim_normal_get_input(editor, &input);
        if (input.key == TB_KEY_ENTER) break;
        inputs[len++] = input;
    }

    *ret_inputs = inputs;
    *ret_inputs_len = len;
    return MLE_OK;
}
