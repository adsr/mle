#include <stdlib.h>
#include <ctype.h>
#include "mle.h"

static int _uscript_cmd_handler(cmd_context_t* ctx);
static void _uscript_aproc_callback(async_proc_t* aproc, char* buf, size_t buf_len);
static void _uscript_register_cmds(uscript_t* uscript);
static int _uscript_parse_msgs(uscript_t* uscript);
static int _uscript_do_convo(uscript_t* uscript, cmd_context_t* ctx);
static int _uscript_parse_msg(uscript_t* uscript, char* line, size_t line_len);
static int _uscript_destroy_msg(uscript_t* uscript, uscript_msg_t* msg);
static int _uscript_read_until_msg(uscript_t* uscript);
static int _uscript_run_cmd_and_send_response(uscript_t* uscript, uscript_msg_t* msg);
static int _uscript_write_request(uscript_t* uscript, cmd_context_t* ctx, char** ret_id);
static int _uscript_write_response(uscript_t* uscript, uscript_msg_t* msg, int rc, char** retvar, char** retvar_name, int* retvar_is_num, int retvar_count);
static int _uscript_write_to_wpipe(uscript_t* uscript, str_t* str);
static void _uscript_url_encode(char* val, str_t* str);
static void _uscript_url_decode(char* val);
static unsigned char _uscript_hexchars[] = "0123456789ABCDEF";

#define MLE_USCRIPT_PTR_TO_STR(pbuf, pptr) \
    snprintf((pbuf), sizeof(pbuf), "%llx", (unsigned long long)(pptr))

#define MLE_USCRIPT_STR_TO_PTR(ptype, pstr) \
    (ptype)strtoull((pstr), NULL, 16)

// Run uscript aproc
uscript_t* uscript_run(editor_t* editor, char* cmd) {
    uscript_t* uscript;
    uscript = calloc(1, sizeof(uscript_t));
    uscript->editor = editor;
    uscript->aproc = async_proc_new(editor, uscript, &(uscript->aproc), cmd, 1, 0, _uscript_aproc_callback);
    if (!uscript->aproc) {
        free(uscript);
        return NULL;
    }
    return uscript;
}

// Destroy uscript
int uscript_destroy(uscript_t* uscript) {
    uscript_msg_t* msg;
    uscript_msg_t* msg_tmp;
    async_proc_destroy(uscript->aproc);
    DL_FOREACH_SAFE(uscript->msgs, msg, msg_tmp) {
        _uscript_destroy_msg(uscript, msg);
    }
    // TODO editor_unregister_cmds we own
    str_free(&uscript->readbuf);
    free(uscript);
    return MLE_OK;
}

// Run a cmd registered by a uscript
static int _uscript_cmd_handler(cmd_context_t* ctx) {
    uscript_t* uscript;
    int rc;
    if (!ctx->cmd->udata) return MLE_ERR;
    uscript = (uscript_t*)ctx->cmd->udata;
    rc = _uscript_do_convo(uscript, ctx);
    if (uscript->has_internal_err) {
        // Commit suicide
        DL_DELETE(ctx->editor->uscripts, uscript);
        uscript_destroy(uscript);
    }
    return rc;
}

// Read messages from uscript
// The only "unprovoked" messages we expect from a uscript is editor_register_cmd
static void _uscript_aproc_callback(async_proc_t* aproc, char* buf, size_t buf_len) {
    uscript_t* uscript;
    if (!aproc->owner) return;
    uscript = (uscript_t*)aproc->owner;
    str_append_len(&uscript->readbuf, buf, buf_len);
    _uscript_parse_msgs(uscript);
    _uscript_register_cmds(uscript);
}

// Register cmds if any
static void _uscript_register_cmds(uscript_t* uscript) {
    uscript_msg_t* msg;
    uscript_msg_t* msg_tmp;
    cmd_t cmd = {0};
    DL_FOREACH_SAFE(uscript->msgs, msg, msg_tmp) {
        if (!msg->is_request
            || msg->params_len < 1
            || strcmp(msg->cmd_name, "editor_register_cmd") != 0
        ) {
            continue;
        }
        cmd.name = msg->params[0];
        cmd.func = _uscript_cmd_handler;
        cmd.udata = uscript;
        editor_register_cmd(uscript->editor, &cmd);
        _uscript_write_response(uscript, msg, 0, NULL, NULL, NULL, 0);
        _uscript_destroy_msg(uscript, msg);
    }
}

// Parse out messages from readbuf
static int _uscript_parse_msgs(uscript_t* uscript) {
    char* newline;
    size_t linelen;
    str_t* readbuf;
    int num_parsed;

    readbuf = &uscript->readbuf;
    while ((newline = memmem(readbuf->data, readbuf->len, "\n", 1)) != NULL) {
        linelen = newline - readbuf->data;
        num_parsed += _uscript_parse_msg(uscript, readbuf->data, linelen);
        memmove(readbuf->data, newline+1, (readbuf->len)-linelen+1);
        readbuf->len -= linelen+1;
    }
    return num_parsed;
}

// Send request to uscript, handle subrequests, return response rc
// Set has_internal_err if something is broken
static int _uscript_do_convo(uscript_t* uscript, cmd_context_t* ctx) {
    char* req_id;
    uscript_msg_t* msg;
    int rc;
    int is_done;

    req_id = NULL;
    rc = MLE_ERR;
    is_done = 0;

    // Send request to uscript
    if (_uscript_write_request(uscript, ctx, &req_id) == MLE_ERR || !req_id) {
        uscript->has_internal_err = 1;
        return MLE_ERR;
    }
    do {
        // Ensure there is at least 1 msg
        if (!uscript->msgs) {
            if (_uscript_read_until_msg(uscript) == MLE_ERR) {
                uscript->has_internal_err = 1;
                break;
            }
        }
        // Look at msg at head of linked list
        msg = uscript->msgs;
        if (msg->is_response) {
            // Response from uscript
            if (strcmp(req_id, msg->id) == 0) {
                // Id matches orig request; done
                // TODO rc = msg->rc;
                is_done = 1;
            } else {
                // Bad id
                uscript->has_internal_err = 1;
            }
        } else {
            // Request from uscript
            if (_uscript_run_cmd_and_send_response(uscript, msg) == MLE_ERR) {
                uscript->has_internal_err = 1;
            }
        }
        // Destroy msg
        _uscript_destroy_msg(uscript, msg);
    } while (!is_done && !uscript->has_internal_err);

    free(req_id);
    return uscript->has_internal_err ? MLE_ERR : rc;
}

// Parse URL-encoded msg as uscript_msg_t
// Return 1 if we parsed a valid msg, otherwise 0
//    Request format: method=name&params[]=str&params[]=2&id=42
//   Response format: result[rv]=0&result[foo]=bar&result[baz]=quux&error=&id=42
//                    error=Some+error+msg&id=42
static int _uscript_parse_msg(uscript_t* uscript, char* line, size_t line_len) {
    char* cur;
    char* line_stop;
    char* amp;
    char* eq;
    char* key;
    char* val;
    char** tmp;
    size_t key_len;
    size_t val_len;
    uscript_msg_t* msg;

    msg = calloc(1, sizeof(uscript_msg_t));

    cur = line;
    line_stop = line + line_len;
    while (cur < line_stop) {
        amp = memchr(cur, '&', line_stop-cur);
        if (!amp) amp = line_stop;
        eq = memchr(cur, '=', amp-cur);
        if (!eq) break;

        // Parse fields into struct
        key = cur;
        key_len = eq-cur;
        val = eq+1;
        val_len = amp-val;
        if (   (key_len == 2 && strncmp(key, "id", 2) == 0)
            || (key_len == 6 && strncmp(key, "method", 6) == 0)
            || (key_len == 5 && strncmp(key, "error", 5) == 0)
        ) {
            // id, method, error
            tmp = *key == 'i' ? &msg->id
                : *key == 'm' ? &msg->cmd_name
                : &msg->error;
            if (*tmp) free(*tmp);
            *tmp = strndup(val, val_len);
            _uscript_url_decode(*tmp);
        } else if (key_len >= 13
            && strncmp(key, "result%5", 6) == 0 && (*(key+8) == 'B' || *(key+8) == 'b')
            && strncmp(key+key_len-3, "%5", 2) == 0 && (*(key+key_len-1) == 'D' || *(key+key_len-1) == 'd')
        ) {
            // result[key]
            msg->result = realloc(msg->result, sizeof(char*) * (msg->result_len+2));
            msg->result[msg->result_len+0] = strndup(key+9, key_len-12);
            msg->result[msg->result_len+1] = strndup(val, val_len);
            _uscript_url_decode(msg->result[msg->result_len+0]);
            _uscript_url_decode(msg->result[msg->result_len+1]);
            msg->result_len += 2;
        } else if (key_len == 12
            && (strncmp(key, "params%5B%5D", 12) == 0
            ||  strncmp(key, "params%5b%5d", 12) == 0
            ||  strncmp(key, "params%5B%5d", 12) == 0
            ||  strncmp(key, "params%5b%5D", 12) == 0)
        ) {
            // params[]
            msg->params = realloc(msg->params, sizeof(char*) * (msg->params_len+1));
            msg->params[msg->params_len] = strndup(val, val_len);
            _uscript_url_decode(msg->params[msg->params_len]);
            msg->params_len += 1;
        }

        cur = amp+1;
    }

    // Ensure fields present
    if (msg->cmd_name && msg->id) {
        msg->is_request = 1;
    } else if ((msg->result || msg->error) && msg->id) {
        msg->is_response = 1;
    } else {
        goto _uscript_parse_msg_failure;
    }

    // Add to linked list
    DL_APPEND(uscript->msgs, msg);
    return 1;

_uscript_parse_msg_failure:
    // Destroy msg
    _uscript_destroy_msg(uscript, msg);
    return 0;
}

// Destroy msg and remove it from linked list if present
static int _uscript_destroy_msg(uscript_t* uscript, uscript_msg_t* msg) {
    int i;
    if (msg->prev) DL_DELETE(uscript->msgs, msg);
    if (msg->id) free(msg->id);
    if (msg->cmd_name) free(msg->cmd_name);
    if (msg->error) free(msg->error);
    for (i = 0; i < msg->result_len; i++) free(msg->result[i]);
    for (i = 0; i < msg->params_len; i++) free(msg->params[i]);
    if (msg->result) free(msg->result);
    if (msg->params) free(msg->params);
    free(msg);
    return MLE_OK;
}

// Read from rpipe until there's a msg
static int _uscript_read_until_msg(uscript_t* uscript) {
    char buf[256];
    fd_set rfds;
    ssize_t nbytes;
    while (!uscript->msgs) {
        FD_ZERO(&rfds);
        FD_SET(uscript->aproc->rfd, &rfds);
        if (select(uscript->aproc->rfd + 1, &rfds, NULL, NULL, NULL) < 1) {
            return MLE_ERR;
        }
        nbytes = read(uscript->aproc->rfd, buf, 256);
        if (nbytes <= 0) return MLE_ERR;
        str_append_len(&uscript->readbuf, buf, nbytes);
        _uscript_parse_msgs(uscript);
    }
    return MLE_OK;
}

// Run command on behalf of uscript and send back response
static int _uscript_run_cmd_and_send_response(uscript_t* uscript, uscript_msg_t* msg) {
#include "uscript_code_gen.inc"
}

// Write request to uscript
//    Request format: method=name&params[]=str&params[]=2&id=42
static int _uscript_write_request(uscript_t* uscript, cmd_context_t* ctx, char** ret_id) {
    int rv;
    char id[64];
    char ptrbuf[32];
    str_t req = {0};

    // Make id
    snprintf(id, 64, "%p-%lu", uscript, uscript->msg_id_counter++);

    // Build request
    str_append(&req, "method=");
    str_append(&req, ctx->cmd->name);
    str_append(&req, "&");
    str_append(&req, "params[mark]=");
        MLE_USCRIPT_PTR_TO_STR(ptrbuf, ctx->cursor->mark);
        str_append(&req, ptrbuf);
        str_append(&req, "&");
    str_append(&req, "params[static_param]=");
        if (ctx->static_param) str_append(&req, ctx->static_param);
        str_append(&req, "&");
    str_append(&req, "id=");
    str_append(&req, id);
    str_append(&req, "\n");

    // Write to wpipe
    if (_uscript_write_to_wpipe(uscript, &req) == MLE_ERR) {
        rv = MLE_ERR;
    } else {
        *ret_id = strdup(id);
        rv = MLE_OK;
    }

    str_free(&req);
    return rv;
}

// Write response to uscript
//   Response format: result[rv]=0&result[foo]=bar&result[baz]=quux&error=&id=42
//                    error=Some+error+msg&id=42
static int _uscript_write_response(uscript_t* uscript, uscript_msg_t* msg, int rc, char** retvar, char** retvar_name, int* retvar_is_num, int retvar_count) {
    str_t tmp = {0};
    str_t res = {0};
    int rv;
    int i;
    char numbuf[32];

    // Build response
    if (rc == MLE_OK) {
        str_append(&res, "result[rc]=");
        sprintf(numbuf, "%d", rc);
        str_append(&res, numbuf);
        str_append(&res, "&");
        for (i = 0; i < retvar_count; i++) {
            str_append(&res, "result[");
            str_append(&res, retvar_name[i]);
            str_append(&res, "]=");
            _uscript_url_encode(retvar[i], &tmp);
            str_append(&res, tmp.data);
            str_append(&res, "&");
        }
    } else {
        str_append(&res, "error=1&");
    }
    str_append(&res, "id=");
    str_append(&res, msg->id);
    str_append(&res, "\n");

    // Write to wpipe
    if (_uscript_write_to_wpipe(uscript, &res) == MLE_ERR) {
        rv = MLE_ERR;
    } else {
        rv = MLE_OK;
    }

    str_free(&res);
    str_free(&tmp);
    return rv;
}

// Write str to uscript wpipe
static int _uscript_write_to_wpipe(uscript_t* uscript, str_t* str) {
    if (str->len != fwrite(str->data, sizeof(char), str->len, uscript->aproc->wpipe)) {
        return MLE_ERR;
    }
    return MLE_OK;
}

// URL-encode `val` into `str`. `str` is reallocated as needed.
// Adapted from https://github.com/php/php-src/blob/master/ext/standard/url.c
static void _uscript_url_encode(char* val, str_t* str) {
    char* from;
    char* to;
    char c;
    size_t val_len;
    char* val_stop;
    val_len = strlen(val);
    val_stop = val + strlen(val);
    str_ensure_cap(str, (val_len*3)+1);
    from = val;
    to = str->data;
    while (from < val_stop) {
        c = *from++;
        if (c == ' ') {
            *to++ = '+';
        } else if (!isalnum(c) && strchr("_-.", c) == NULL) {
            to[0] = '%';
            to[1] = _uscript_hexchars[c >> 4];
            to[2] = _uscript_hexchars[c & 15];
            to += 3;
        } else {
            *to++ = c;
        }
    }
    *to = '\0';
}

// URL-decode `val` in place
// Adapted from https://github.com/php/php-src/blob/master/ext/standard/url.c
static void _uscript_url_decode(char* val) {
    size_t val_len;
    char* from;
    char* to;
    char hex[3];

    val_len = strlen(val);
    from = val;
    to = val;
    hex[2] = '\0';

    while (val_len--) {
        if (*from == '+') {
            *to = ' ';
        } else if (*from == '%' && val_len >= 2
            && isxdigit((int)*(from+1))
            && isxdigit((int)*(from+2))
        ) {
            hex[0] = *(from+1);
            hex[1] = *(from+2);
            *to = (char)strtol(hex, NULL, 16);
            from += 2;
            val_len -= 2;
        } else {
            *to = *from;
        }
        from++;
        to++;
    }
    *to = '\0';
}
