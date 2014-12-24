#include <stdlib.h>
#include "mle.h"

editor_t editor;

static void parse_rc_file();
static void parse_cli_opts(char** argv);

int main(int argc, char** argv) {
    parse_rc_file();
    parse_cli_opts(argv);
    editor_init(&editor);
    editor_loop(&editor);
    editor_deinit(&editor);
    return EXIT_SUCCESS;
}

static void parse_rc_file() {
    // TODO look env MLERC to override rc location
}

static void parse_cli_opts(char** argv) {
    // TODO tab stop, tab to space, file:line#(s) to open
}
