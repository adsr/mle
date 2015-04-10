#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <termbox.h>
#include "uthash.h"
#include "mlbuf/mlbuf.h"
#include "mle.h"

editor_t _editor;

int main(int argc, char** argv) {
    memset(&_editor, 0, sizeof(editor_t));
    setlocale(LC_ALL, "");
    if (editor_init(&_editor, argc, argv) == MLE_OK) {
        tb_init();
        tb_select_input_mode(TB_INPUT_ALT);
        editor_run(&_editor);
        editor_deinit(&_editor);
        tb_shutdown();
    } else {
        editor_deinit(&_editor);
    }
    return _editor.exit_code;
}
