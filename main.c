#include <stdlib.h>
#include <limits.h>
#include <termbox.h>
#include "uthash.h"
#include "mlbuf/mlbuf.h"
#include "mle.h"

editor_t editor;

static void parse_rc_file();
static void parse_cli_opts(char** argv);

int main(int argc, char** argv) {
    parse_rc_file();
    parse_cli_opts(argv);
    tb_init();
    tb_select_input_mode(TB_INPUT_ALT);
    editor_init(&editor);
    editor_loop(&editor);
    editor_deinit(&editor);
    tb_shutdown();
    return EXIT_SUCCESS;
}

// Parse rc file
static void parse_rc_file() {
    // TODO look env MLERC to override rc location
}

static void parse_cli_opts(char** argv) {
    //editor.init_files
    //init_file_t* init_files;
    // TODO tab stop, tab to space, file:line#(s) to open
}
