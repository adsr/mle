#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <uthash.h>
#include "termbox2.h"
#include "mlbuf.h"
#include "mle.h"

editor_t _editor;

int main(int argc, char **argv) {
    memset(&_editor, 0, sizeof(editor_t));
    setlocale(LC_ALL, "");
    if (editor_init(&_editor, argc, argv) == MLE_OK) {
        editor_run(&_editor);
    }
    editor_deinit(&_editor);
    return _editor.exit_code;
}
