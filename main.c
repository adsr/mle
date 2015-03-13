#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <termbox.h>
#include "uthash.h"
#include "mlbuf/mlbuf.h"
#include "mle.h"


int main(int argc, char** argv) {
    editor_t editor;
    memset(&editor, 0, sizeof(editor_t));
    setlocale(LC_ALL, "");
    editor_init(&editor, argc, argv);
    tb_init();
    tb_select_input_mode(TB_INPUT_ALT);
    editor_run(&editor);
    editor_deinit(&editor);
    tb_shutdown();
    return EXIT_SUCCESS;
}
