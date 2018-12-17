#include "test.h"

buffer_t* global_buf;
int* global_udata;

static void callback_fn(buffer_t* buf, baction_t* bac, void* udata) {
    ASSERT("bufp", global_buf, buf);
    ASSERT("udata", (void*)global_udata, udata);
    ASSERT("bac_type", MLBUF_BACTION_TYPE_INSERT, bac->type);
    ASSERT("bac_buf", buf, bac->buffer);
    ASSERT("bac_start_line", buf->first_line, bac->start_line);
    ASSERT("bac_start_line_index", 0, bac->start_line_index);
    ASSERT("bac_start_col", 0, bac->start_col);
    ASSERT("bac_maybe_end_line", buf->first_line->next, bac->maybe_end_line);
    ASSERT("bac_maybe_end_line_index", 1, bac->maybe_end_line_index);
    ASSERT("bac_maybe_end_col", 2, bac->maybe_end_col);
    ASSERT("bac_byte_delta", 5, bac->byte_delta);
    ASSERT("bac_char_delta", 5, bac->char_delta);
    ASSERT("bac_line_delta", 1, bac->line_delta);
    ASSERT("bac_data", 0, strncmp("te\nst", bac->data, bac->data_len));
}

MAIN("hello\nworld",
    int udata;
    udata = 42;
    global_buf = buf;
    global_udata = &udata;
    buffer_set_callback(buf, callback_fn, (void*)global_udata);
    buffer_insert(buf, 0, "te\nst", 5, NULL);
)
