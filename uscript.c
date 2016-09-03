#include "jsmn.h"
#include "mle.h"

// TODO use urlencode/decode instead of json
//      method=foo&params[]=baz&params[]=42&id=bar
//      result[foo]=bar&result[baz]=quux&error=0&id=thud

static int uscript_cmd_handler(cmd_context_t* ctx);
static void _uscript_aproc_callback(async_proc_t* aproc, char* buf, size_t buf_len);
static void _uscript_register_cmds(uscript_t* uscript);
static int _uscript_parse_msgs(uscript_t* uscript);
static int _uscript_do_convo(uscript_t* uscript, cmd_context_t* ctx);
static int _uscript_parse_msg(uscript_t* uscript, char* json, jsmntok_t* t, int jsrc);
static int _uscript_destroy_msg(uscript_t* uscript, uscript_msg_t* msg);
static int _uscript_write_request(uscript_t* uscript, cmd_context_t* ctx, char** ret_id);
static int _uscript_read_until_msg(uscript_t* uscript);
static int _uscript_run_cmd_and_send_response(uscript_t* uscript, uscript_msg_t* msg);
static int _uscript_write_response(uscript_t* uscript, uscript_msg_t* msg, int rc, char** retvar, char** retvar_name, int* retvar_is_num, int retvar_count);
static int _uscript_write_to_wpipe(uscript_t* uscript, str_t* str);
static int _uscript_jsmn_strcmp(const char *json, jsmntok_t *tok, const char *s);
static void _uscript_str_append_json_str(str_t* str, char* c);

#define MLE_USCRIPT_PTR_TO_STR(pbuf, pptr) \
    snprintf((pbuf), sizeof(pbuf), "%llx", (unsigned long long)(pptr))

#define MLE_USCRIPT_STR_TO_PTR(ptype, pstr) \
    (ptype)strtoull((pstr), NULL, 16)

// Run uscript aproc
uscript_t* uscript_run(editor_t* editor, char* cmd) {
    uscript_t* uscript;
    uscript = calloc(1, sizeof(uscript_t));
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
    str_free(&uscript->readbuf);
    free(uscript);
    return MLE_OK;
}

// Run a cmd registered by a uscript
static int uscript_cmd_handler(cmd_context_t* ctx) {
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
}

// Parse out json messages from readbuf
static int _uscript_parse_msgs(uscript_t* uscript) {
    jsmn_parser jsmn;
    jsmntok_t jstoks[32];
    int jsrc;
    char* newline;
    size_t jslen;
    str_t* readbuf;
    int num_parsed;

    readbuf = &uscript->readbuf;
    jsmn_init(&jsmn);
    while ((newline = memmem(readbuf->data, readbuf->len, "\n", 1)) != NULL) {
        jslen = newline - readbuf->data;
        jsrc = jsmn_parse(&jsmn, readbuf->data, jslen, jstoks, 32);
        num_parsed += _uscript_parse_msg(uscript, readbuf->data, jstoks, jsrc);
        memmove(readbuf->data, newline+1, (readbuf->len - jslen)+1);
        readbuf->len -= jslen+1;
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

// Parse json as uscript_msg_t
// Return 1 if we parsed a valid msg, otherwise 0
// Message format is JSON-RPC 1.0:
//     {"method":"blah", "params":["str", 1, "whatever"], "id":"blah"}
//     {"result":"blah", "error":"blah", "id":"blah"}
static int _uscript_parse_msg(uscript_t* uscript, char* json, jsmntok_t* t, int jsrc) {
    int i;
    int j;
    int s;
    int e;
    uscript_msg_t* msg;

    // Ensure object with at least 7 tokens
    // 1  token for object
    // 2  tokens for method
    // 2+ tokens for params
    // 2  tokens for id
    if (jsrc < 7 || t[0].type != JSMN_OBJECT) {
        return 0;
    }

    // Parse fields
    msg = calloc(1, sizeof(uscript_msg_t));
    for (i = 1; i < jsrc; i++) {
        if (i+1 < jsrc) {
            s = t[i+1].start;
            e = t[i+1].end;
        } else {
            s = e = 0;
        }
        if (       _uscript_jsmn_strcmp(json, t+i,  "id") == 0) {
            asprintf(&msg->id,       "%.*s", e-s, json+s);
        } else if (_uscript_jsmn_strcmp(json, t+i,  "method") == 0) {
            asprintf(&msg->cmd_name, "%.*s", e-s, json+s);
        } else if (_uscript_jsmn_strcmp(json, t+i,  "result") == 0) {
            asprintf(&msg->result,   "%.*s", e-s, json+s);
        } else if (_uscript_jsmn_strcmp(json, t+i,  "error") == 0) {
            asprintf(&msg->error,    "%.*s", e-s, json+s);
        } else if (_uscript_jsmn_strcmp(json, t+i,  "params") == 0) {
            if (t[i+1].type != JSMN_ARRAY) goto _uscript_parse_msg_failure;
            msg->params_len = t[i+1].size;
            if (msg->params_len  > 0) {
                msg->params = malloc(msg->params_len * sizeof(char*));
                for (j = 0; j < msg->params_len; j++) {
                    s = t[i+j+2].start;
                    e = t[i+j+2].end;
                    asprintf(msg->params+j, "%.*s", e-s, json+s);
                }
            }
        }
    }

    // Ensure fields present
    if (msg->cmd_name && msg->params && msg->id) {
        msg->is_request = 1;
    } else if (msg->result && msg->error && msg->id) {
        msg->is_response = 1;
    } else {
        goto _uscript_parse_msg_failure;
    }

    // Add to linked list
    DL_APPEND(uscript->msgs, msg);
    uscript->msg_count += 1;
    return 1;

_uscript_parse_msg_failure:
    // Destroy msg
    _uscript_destroy_msg(uscript, msg);
    return 0;
}

// Destroy msg and remove it from linked list if present
static int _uscript_destroy_msg(uscript_t* uscript, uscript_msg_t* msg) {
    int i;
    DL_DELETE(uscript->msgs, msg);
    uscript->msg_count -= 1;
    if (msg->cmd_name) free(msg->cmd_name);
    if (msg->result) free(msg->result);
    if (msg->id) free(msg->id);
    if (msg->error) free(msg->error);
    for (i = 0; i < msg->params_len; i++) free(msg->params[i]);
    if (msg->params) free(msg->params);
    free(msg);
    return MLE_OK;
}

// Write request to uscript
// Format is: {"method":"blah", "params":["str", 1, "whatever"], "id":"blah"}
static int _uscript_write_request(uscript_t* uscript, cmd_context_t* ctx, char** ret_id) {
    char id[64];
    //char ptrbuf[32];
    str_t req;

    // Make id
    snprintf(id, 64, "%p-%lu", uscript, uscript->msg_counter++);

    // Build json request
    str_append(&req, "{\"method\":");
    _uscript_str_append_json_str(&req, ctx->cmd->name);
    str_append(&req, ",\"id\":");
    str_append(&req, id);
    str_append(&req, "\"");
    str_append(&req, ",\"params\":[{");
    // TODO mark, static_param, bline->data
    //str_append(&req, "\"mark\":"); _uscript_str_append_json_str_voidptr(&req, (void*)ctx->cursor->mark);
    //str_append(&req, ",\"static_param\":"); _uscript_str_append_json_str(&req, ctx->static_param);
    //str_append(&req, ",\"line\":"); _uscript_str_append_json_str_len(&req, ctx->bline->data, ctx->bline->data_len);
    str_append(&req, "}]");

    // Write to wpipe
    if (_uscript_write_to_wpipe(uscript, &req) == MLE_ERR) {
        str_free(&req);
        return MLE_ERR;
    }

    str_free(&req);
    *ret_id = strdup(id);
    return MLE_OK;
}

// Read from rpipe until there's a msg
static int _uscript_read_until_msg(uscript_t* uscript) {
    char buf[256];
    fd_set rfds;
    size_t nbytes;
    while (!uscript->msgs) {
        FD_ZERO(&rfds);
        FD_SET(uscript->aproc->rfd, &rfds);
        if (select(uscript->aproc->rfd + 1, &rfds, NULL, NULL, NULL) < 1) {
            return MLE_ERR;
        }
        nbytes = fread(buf, sizeof(char), 256, uscript->aproc->rpipe);
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

// Write response to uscript
// Format is: {"result":{"var":"", "var2":"..."}, "error":"blah", "id":"blah"}
static int _uscript_write_response(uscript_t* uscript, uscript_msg_t* msg, int rc, char** retvar, char** retvar_name, int* retvar_is_num, int retvar_count) {
    str_t res;
    int i;

    // Build response
    if (rc == MLE_OK) {
        str_append(&res, "{\"result\":{");
        for (i = 0; i < retvar_count; i++) {
            _uscript_str_append_json_str(&res, retvar_name[i]);
            str_append(&res, ":");
            if (retvar_is_num[i]) {
                str_append(&res, retvar[i]);
            } else {
                _uscript_str_append_json_str(&res, retvar[i]);
            }
            if (i + 1 < retvar_count) {
                str_append(&res, ",");
            }
        }
        str_append(&res, "},\"error\":0,");
    } else {
        str_append(&res, "{\"result\":null,\"error\":1,");
    }
    str_append(&res, "\"id\":");
    _uscript_str_append_json_str(&res, msg->id);
    str_append(&res, "}");

    // Write to wpipe
    _uscript_write_to_wpipe(uscript, &res);
    if (_uscript_write_to_wpipe(uscript, &res) == MLE_ERR) {
        str_free(&res);
        return MLE_ERR;
    }

    str_free(&res);
    return MLE_OK;
}

// Write str to uscript wpipe
static int _uscript_write_to_wpipe(uscript_t* uscript, str_t* str) {
    if (str->len != fwrite(str->data, sizeof(char), str->len, uscript->aproc->wpipe)) {
        return MLE_ERR;
    }
    return MLE_OK;
}

// Adapted from https://github.com/zserge/jsmn/blob/master/example/simple.c
static int _uscript_jsmn_strcmp(const char *json, jsmntok_t *tok, const char *s) {
    if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
        strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
        return 0;
    }
    return -1;
}

static void _uscript_str_append_json_str(str_t* str, char* c) {
    //
}
