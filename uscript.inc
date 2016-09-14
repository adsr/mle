    mark_t* tmp_markp_0 = NULL;
    mark_t* tmp_markp_1 = NULL;
    char tmp_char_0 = 0;
    char* tmp_charp_0 = NULL;
    bint_t tmp_bint_0 = 0;
    bint_t tmp_bint_1 = 0;
    bint_t tmp_bint_2 = 0;
    bline_t* tmp_blinep_0 = NULL;
    int tmp_int_0 = 0;
    uint32_t tmp_uint_0 = 0;
    int i = 0;
    char ptrbuf[32];
    int rc = MLE_ERR;
    int rf = MLE_ERR;
    int retvar_count = 0;
    char* retvar_name[3];
    char* retvar[3];
    do { if (0) {
    } else if (strcmp(msg->cmd_name, "mark_clone") == 0) {
        if (msg->params_len != 1) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        rc = mark_clone(tmp_markp_0, &tmp_markp_1);
        retvar_name[0] = "mark";
        MLE_USCRIPT_PTR_TO_STR(ptrbuf, tmp_markp_1); retvar[0] = strdup(ptrbuf);
        retvar_count = 1;
    } else if (strcmp(msg->cmd_name, "mark_clone_w_letter") == 0) {
        if (msg->params_len != 2) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_char_0 = (char)strtoll(msg->params[1], NULL, 10);
        rc = mark_clone_w_letter(tmp_markp_0, tmp_char_0, &tmp_markp_1);
        retvar_name[0] = "mark";
        MLE_USCRIPT_PTR_TO_STR(ptrbuf, tmp_markp_1); retvar[0] = strdup(ptrbuf);
        retvar_count = 1;
    } else if (strcmp(msg->cmd_name, "mark_insert_before") == 0) {
        if (msg->params_len != 3) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_charp_0 = msg->params[1];
        tmp_bint_0 = (bint_t)strtoll(msg->params[2], NULL, 10);
        rc = mark_insert_before(tmp_markp_0, tmp_charp_0, tmp_bint_0);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_insert_after") == 0) {
        if (msg->params_len != 3) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_charp_0 = msg->params[1];
        tmp_bint_0 = (bint_t)strtoll(msg->params[2], NULL, 10);
        rc = mark_insert_after(tmp_markp_0, tmp_charp_0, tmp_bint_0);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_delete_before") == 0) {
        if (msg->params_len != 2) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_bint_0 = (bint_t)strtoll(msg->params[1], NULL, 10);
        rc = mark_delete_before(tmp_markp_0, tmp_bint_0);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_delete_after") == 0) {
        if (msg->params_len != 2) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_bint_0 = (bint_t)strtoll(msg->params[1], NULL, 10);
        rc = mark_delete_after(tmp_markp_0, tmp_bint_0);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_move_to") == 0) {
        if (msg->params_len != 3) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_bint_0 = (bint_t)strtoll(msg->params[1], NULL, 10);
        tmp_bint_1 = (bint_t)strtoll(msg->params[2], NULL, 10);
        rc = mark_move_to(tmp_markp_0, tmp_bint_0, tmp_bint_1);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_move_to_w_bline") == 0) {
        if (msg->params_len != 3) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_blinep_0 = MLE_USCRIPT_STR_TO_PTR(bline_t*, msg->params[1]);
        tmp_bint_0 = (bint_t)strtoll(msg->params[2], NULL, 10);
        rc = mark_move_to_w_bline(tmp_markp_0, tmp_blinep_0, tmp_bint_0);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_move_by") == 0) {
        if (msg->params_len != 2) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_bint_0 = (bint_t)strtoll(msg->params[1], NULL, 10);
        rc = mark_move_by(tmp_markp_0, tmp_bint_0);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_move_vert") == 0) {
        if (msg->params_len != 2) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_bint_0 = (bint_t)strtoll(msg->params[1], NULL, 10);
        rc = mark_move_vert(tmp_markp_0, tmp_bint_0);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_move_bol") == 0) {
        if (msg->params_len != 1) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        rc = mark_move_bol(tmp_markp_0);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_move_eol") == 0) {
        if (msg->params_len != 1) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        rc = mark_move_eol(tmp_markp_0);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_move_col") == 0) {
        if (msg->params_len != 2) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_bint_0 = (bint_t)strtoll(msg->params[1], NULL, 10);
        rc = mark_move_col(tmp_markp_0, tmp_bint_0);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_move_beginning") == 0) {
        if (msg->params_len != 1) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        rc = mark_move_beginning(tmp_markp_0);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_move_end") == 0) {
        if (msg->params_len != 1) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        rc = mark_move_end(tmp_markp_0);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_move_offset") == 0) {
        if (msg->params_len != 2) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_bint_0 = (bint_t)strtoll(msg->params[1], NULL, 10);
        rc = mark_move_offset(tmp_markp_0, tmp_bint_0);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_find_next_str") == 0) {
        if (msg->params_len != 3) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_charp_0 = msg->params[1];
        tmp_bint_0 = (bint_t)strtoll(msg->params[2], NULL, 10);
        rc = mark_find_next_str(tmp_markp_0, tmp_charp_0, tmp_bint_0, &tmp_blinep_0, &tmp_bint_1, &tmp_bint_2);
        retvar_name[0] = "line";
        MLE_USCRIPT_PTR_TO_STR(ptrbuf, tmp_blinep_0); retvar[0] = strdup(ptrbuf);
        retvar_name[1] = "col";
        asprintf(&retvar[1], "%lld", (long long)tmp_bint_1);
        retvar_name[2] = "num_chars";
        asprintf(&retvar[2], "%lld", (long long)tmp_bint_2);
        retvar_count = 3;
    } else if (strcmp(msg->cmd_name, "mark_find_prev_str") == 0) {
        if (msg->params_len != 3) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_charp_0 = msg->params[1];
        tmp_bint_0 = (bint_t)strtoll(msg->params[2], NULL, 10);
        rc = mark_find_prev_str(tmp_markp_0, tmp_charp_0, tmp_bint_0, &tmp_blinep_0, &tmp_bint_1, &tmp_bint_2);
        retvar_name[0] = "line";
        MLE_USCRIPT_PTR_TO_STR(ptrbuf, tmp_blinep_0); retvar[0] = strdup(ptrbuf);
        retvar_name[1] = "col";
        asprintf(&retvar[1], "%lld", (long long)tmp_bint_1);
        retvar_name[2] = "num_chars";
        asprintf(&retvar[2], "%lld", (long long)tmp_bint_2);
        retvar_count = 3;
    } else if (strcmp(msg->cmd_name, "mark_find_next_re") == 0) {
        if (msg->params_len != 3) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_charp_0 = msg->params[1];
        tmp_bint_0 = (bint_t)strtoll(msg->params[2], NULL, 10);
        rc = mark_find_next_re(tmp_markp_0, tmp_charp_0, tmp_bint_0, &tmp_blinep_0, &tmp_bint_1, &tmp_bint_2);
        retvar_name[0] = "line";
        MLE_USCRIPT_PTR_TO_STR(ptrbuf, tmp_blinep_0); retvar[0] = strdup(ptrbuf);
        retvar_name[1] = "col";
        asprintf(&retvar[1], "%lld", (long long)tmp_bint_1);
        retvar_name[2] = "num_chars";
        asprintf(&retvar[2], "%lld", (long long)tmp_bint_2);
        retvar_count = 3;
    } else if (strcmp(msg->cmd_name, "mark_find_prev_re") == 0) {
        if (msg->params_len != 3) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_charp_0 = msg->params[1];
        tmp_bint_0 = (bint_t)strtoll(msg->params[2], NULL, 10);
        rc = mark_find_prev_re(tmp_markp_0, tmp_charp_0, tmp_bint_0, &tmp_blinep_0, &tmp_bint_1, &tmp_bint_2);
        retvar_name[0] = "line";
        MLE_USCRIPT_PTR_TO_STR(ptrbuf, tmp_blinep_0); retvar[0] = strdup(ptrbuf);
        retvar_name[1] = "col";
        asprintf(&retvar[1], "%lld", (long long)tmp_bint_1);
        retvar_name[2] = "num_chars";
        asprintf(&retvar[2], "%lld", (long long)tmp_bint_2);
        retvar_count = 3;
    } else if (strcmp(msg->cmd_name, "mark_find_bracket_pair") == 0) {
        if (msg->params_len != 2) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_bint_0 = (bint_t)strtoll(msg->params[1], NULL, 10);
        rc = mark_find_bracket_pair(tmp_markp_0, tmp_bint_0, &tmp_blinep_0, &tmp_bint_1, &tmp_bint_2);
        retvar_name[0] = "line";
        MLE_USCRIPT_PTR_TO_STR(ptrbuf, tmp_blinep_0); retvar[0] = strdup(ptrbuf);
        retvar_name[1] = "col";
        asprintf(&retvar[1], "%lld", (long long)tmp_bint_1);
        retvar_name[2] = "brkt";
        asprintf(&retvar[2], "%lld", (long long)tmp_bint_2);
        retvar_count = 3;
    } else if (strcmp(msg->cmd_name, "mark_find_bracket_top") == 0) {
        if (msg->params_len != 2) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_bint_0 = (bint_t)strtoll(msg->params[1], NULL, 10);
        rc = mark_find_bracket_top(tmp_markp_0, tmp_bint_0, &tmp_blinep_0, &tmp_bint_1, &tmp_bint_2);
        retvar_name[0] = "line";
        MLE_USCRIPT_PTR_TO_STR(ptrbuf, tmp_blinep_0); retvar[0] = strdup(ptrbuf);
        retvar_name[1] = "col";
        asprintf(&retvar[1], "%lld", (long long)tmp_bint_1);
        retvar_name[2] = "brkt";
        asprintf(&retvar[2], "%lld", (long long)tmp_bint_2);
        retvar_count = 3;
    } else if (strcmp(msg->cmd_name, "mark_move_next_str") == 0) {
        if (msg->params_len != 3) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_charp_0 = msg->params[1];
        tmp_bint_0 = (bint_t)strtoll(msg->params[2], NULL, 10);
        rc = mark_move_next_str(tmp_markp_0, tmp_charp_0, tmp_bint_0);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_move_prev_str") == 0) {
        if (msg->params_len != 3) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_charp_0 = msg->params[1];
        tmp_bint_0 = (bint_t)strtoll(msg->params[2], NULL, 10);
        rc = mark_move_prev_str(tmp_markp_0, tmp_charp_0, tmp_bint_0);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_move_next_re") == 0) {
        if (msg->params_len != 3) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_charp_0 = msg->params[1];
        tmp_bint_0 = (bint_t)strtoll(msg->params[2], NULL, 10);
        rc = mark_move_next_re(tmp_markp_0, tmp_charp_0, tmp_bint_0);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_move_prev_re") == 0) {
        if (msg->params_len != 3) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_charp_0 = msg->params[1];
        tmp_bint_0 = (bint_t)strtoll(msg->params[2], NULL, 10);
        rc = mark_move_prev_re(tmp_markp_0, tmp_charp_0, tmp_bint_0);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_move_bracket_pair") == 0) {
        if (msg->params_len != 2) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_bint_0 = (bint_t)strtoll(msg->params[1], NULL, 10);
        rc = mark_move_bracket_pair(tmp_markp_0, tmp_bint_0);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_move_bracket_top") == 0) {
        if (msg->params_len != 2) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_bint_0 = (bint_t)strtoll(msg->params[1], NULL, 10);
        rc = mark_move_bracket_top(tmp_markp_0, tmp_bint_0);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_move_next_str_ex") == 0) {
        if (msg->params_len != 3) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_charp_0 = msg->params[1];
        tmp_bint_0 = (bint_t)strtoll(msg->params[2], NULL, 10);
        rc = mark_move_next_str_ex(tmp_markp_0, tmp_charp_0, tmp_bint_0, &tmp_blinep_0, &tmp_bint_1, &tmp_bint_2);
        retvar_name[0] = "line";
        MLE_USCRIPT_PTR_TO_STR(ptrbuf, tmp_blinep_0); retvar[0] = strdup(ptrbuf);
        retvar_name[1] = "col";
        asprintf(&retvar[1], "%lld", (long long)tmp_bint_1);
        retvar_name[2] = "num_chars";
        asprintf(&retvar[2], "%lld", (long long)tmp_bint_2);
        retvar_count = 3;
    } else if (strcmp(msg->cmd_name, "mark_move_prev_str_ex") == 0) {
        if (msg->params_len != 3) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_charp_0 = msg->params[1];
        tmp_bint_0 = (bint_t)strtoll(msg->params[2], NULL, 10);
        rc = mark_move_prev_str_ex(tmp_markp_0, tmp_charp_0, tmp_bint_0, &tmp_blinep_0, &tmp_bint_1, &tmp_bint_2);
        retvar_name[0] = "line";
        MLE_USCRIPT_PTR_TO_STR(ptrbuf, tmp_blinep_0); retvar[0] = strdup(ptrbuf);
        retvar_name[1] = "col";
        asprintf(&retvar[1], "%lld", (long long)tmp_bint_1);
        retvar_name[2] = "num_chars";
        asprintf(&retvar[2], "%lld", (long long)tmp_bint_2);
        retvar_count = 3;
    } else if (strcmp(msg->cmd_name, "mark_move_next_re_ex") == 0) {
        if (msg->params_len != 3) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_charp_0 = msg->params[1];
        tmp_bint_0 = (bint_t)strtoll(msg->params[2], NULL, 10);
        rc = mark_move_next_re_ex(tmp_markp_0, tmp_charp_0, tmp_bint_0, &tmp_blinep_0, &tmp_bint_1, &tmp_bint_2);
        retvar_name[0] = "line";
        MLE_USCRIPT_PTR_TO_STR(ptrbuf, tmp_blinep_0); retvar[0] = strdup(ptrbuf);
        retvar_name[1] = "col";
        asprintf(&retvar[1], "%lld", (long long)tmp_bint_1);
        retvar_name[2] = "num_chars";
        asprintf(&retvar[2], "%lld", (long long)tmp_bint_2);
        retvar_count = 3;
    } else if (strcmp(msg->cmd_name, "mark_move_prev_re_ex") == 0) {
        if (msg->params_len != 3) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_charp_0 = msg->params[1];
        tmp_bint_0 = (bint_t)strtoll(msg->params[2], NULL, 10);
        rc = mark_move_prev_re_ex(tmp_markp_0, tmp_charp_0, tmp_bint_0, &tmp_blinep_0, &tmp_bint_1, &tmp_bint_2);
        retvar_name[0] = "line";
        MLE_USCRIPT_PTR_TO_STR(ptrbuf, tmp_blinep_0); retvar[0] = strdup(ptrbuf);
        retvar_name[1] = "col";
        asprintf(&retvar[1], "%lld", (long long)tmp_bint_1);
        retvar_name[2] = "num_chars";
        asprintf(&retvar[2], "%lld", (long long)tmp_bint_2);
        retvar_count = 3;
    } else if (strcmp(msg->cmd_name, "mark_move_bracket_pair_ex") == 0) {
        if (msg->params_len != 2) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_bint_0 = (bint_t)strtoll(msg->params[1], NULL, 10);
        rc = mark_move_bracket_pair_ex(tmp_markp_0, tmp_bint_0, &tmp_blinep_0, &tmp_bint_1, &tmp_bint_2);
        retvar_name[0] = "line";
        MLE_USCRIPT_PTR_TO_STR(ptrbuf, tmp_blinep_0); retvar[0] = strdup(ptrbuf);
        retvar_name[1] = "col";
        asprintf(&retvar[1], "%lld", (long long)tmp_bint_1);
        retvar_name[2] = "num_chars";
        asprintf(&retvar[2], "%lld", (long long)tmp_bint_2);
        retvar_count = 3;
    } else if (strcmp(msg->cmd_name, "mark_move_bracket_top_ex") == 0) {
        if (msg->params_len != 2) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_bint_0 = (bint_t)strtoll(msg->params[1], NULL, 10);
        rc = mark_move_bracket_top_ex(tmp_markp_0, tmp_bint_0, &tmp_blinep_0, &tmp_bint_1, &tmp_bint_2);
        retvar_name[0] = "line";
        MLE_USCRIPT_PTR_TO_STR(ptrbuf, tmp_blinep_0); retvar[0] = strdup(ptrbuf);
        retvar_name[1] = "col";
        asprintf(&retvar[1], "%lld", (long long)tmp_bint_1);
        retvar_name[2] = "num_chars";
        asprintf(&retvar[2], "%lld", (long long)tmp_bint_2);
        retvar_count = 3;
    } else if (strcmp(msg->cmd_name, "mark_get_offset") == 0) {
        if (msg->params_len != 1) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        rc = mark_get_offset(tmp_markp_0, &tmp_bint_0);
        retvar_name[0] = "offset";
        asprintf(&retvar[0], "%lld", (long long)tmp_bint_0);
        retvar_count = 1;
    } else if (strcmp(msg->cmd_name, "mark_delete_between_mark") == 0) {
        if (msg->params_len != 2) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_markp_1 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[1]);
        rc = mark_delete_between_mark(tmp_markp_0, tmp_markp_1);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_get_between_mark") == 0) {
        if (msg->params_len != 2) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_markp_1 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[1]);
        rc = mark_get_between_mark(tmp_markp_0, tmp_markp_1, &tmp_charp_0, &tmp_bint_0);
        retvar_name[0] = "str";
        retvar[0] = strdup(tmp_charp_0);
        retvar_name[1] = "str_len";
        asprintf(&retvar[1], "%lld", (long long)tmp_bint_0);
        retvar_count = 2;
    } else if (strcmp(msg->cmd_name, "mark_is_lt") == 0) {
        if (msg->params_len != 2) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_markp_1 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[1]);
        rc = mark_is_lt(tmp_markp_0, tmp_markp_1);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_is_gt") == 0) {
        if (msg->params_len != 2) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_markp_1 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[1]);
        rc = mark_is_gt(tmp_markp_0, tmp_markp_1);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_is_eq") == 0) {
        if (msg->params_len != 2) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_markp_1 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[1]);
        rc = mark_is_eq(tmp_markp_0, tmp_markp_1);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_is_gte") == 0) {
        if (msg->params_len != 2) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_markp_1 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[1]);
        rc = mark_is_gte(tmp_markp_0, tmp_markp_1);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_is_lte") == 0) {
        if (msg->params_len != 2) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_markp_1 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[1]);
        rc = mark_is_lte(tmp_markp_0, tmp_markp_1);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_join") == 0) {
        if (msg->params_len != 2) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_markp_1 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[1]);
        rc = mark_join(tmp_markp_0, tmp_markp_1);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_swap_with_mark") == 0) {
        if (msg->params_len != 2) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_markp_1 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[1]);
        rc = mark_swap_with_mark(tmp_markp_0, tmp_markp_1);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_is_at_eol") == 0) {
        if (msg->params_len != 1) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        rc = mark_is_at_eol(tmp_markp_0);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_is_at_bol") == 0) {
        if (msg->params_len != 1) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        rc = mark_is_at_bol(tmp_markp_0);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_is_at_word_bound") == 0) {
        if (msg->params_len != 2) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        tmp_int_0 = (int)strtoll(msg->params[1], NULL, 10);
        rc = mark_is_at_word_bound(tmp_markp_0, tmp_int_0);
        retvar_count = 0;
    } else if (strcmp(msg->cmd_name, "mark_get_char_after") == 0) {
        if (msg->params_len != 1) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        rc = mark_get_char_after(tmp_markp_0, &tmp_uint_0);
        retvar_name[0] = "char";
        asprintf(&retvar[0], "%llu", (unsigned long long)tmp_uint_0);
        retvar_count = 1;
    } else if (strcmp(msg->cmd_name, "mark_get_char_before") == 0) {
        if (msg->params_len != 1) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        rc = mark_get_char_before(tmp_markp_0, &tmp_uint_0);
        retvar_name[0] = "char";
        asprintf(&retvar[0], "%llu", (unsigned long long)tmp_uint_0);
        retvar_count = 1;
    } else if (strcmp(msg->cmd_name, "mark_destroy") == 0) {
        if (msg->params_len != 1) break;
        tmp_markp_0 = MLE_USCRIPT_STR_TO_PTR(mark_t*, msg->params[0]);
        rc = mark_destroy(tmp_markp_0);
        retvar_count = 0;
    } } while(0);
    rf = _uscript_write_response(uscript, msg, rc, retvar, retvar_name, retvar_count);
    for (i = 0; i < retvar_count; i++) free(retvar[i]);
    return rf;
