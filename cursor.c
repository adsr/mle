#include <string.h>
#include <ctype.h>
#include "mle.h"

static int cursor_uncut_ex(cursor_t *cursor, int editor_wide);

// Clone cursor
int cursor_clone(cursor_t *cursor, int use_srules, cursor_t **ret_clone) {
    cursor_t *clone;
    bview_add_cursor(cursor->bview, cursor->mark->bline, cursor->mark->col, &clone);
    if (cursor->is_anchored) {
        cursor_toggle_anchor(clone, use_srules);
        mark_join(clone->anchor, cursor->anchor);
    }
    *ret_clone = clone;
    return MLE_OK;
}

// Remove cursor
int cursor_destroy(cursor_t *cursor) {
    return bview_remove_cursor(cursor->bview, cursor);
}

// Select by mark
int cursor_select_between(cursor_t *cursor, mark_t *a, mark_t *b, int use_srules) {
    cursor_drop_anchor(cursor, use_srules);
    if (mark_is_lt(a, b)) {
        mark_join(cursor->mark, a);
        mark_join(cursor->anchor, b);
    } else {
        mark_join(cursor->mark, b);
        mark_join(cursor->anchor, a);
    }
    return MLE_OK;
}

// Toggle cursor anchor
int cursor_toggle_anchor(cursor_t *cursor, int use_srules) {
    if (!cursor->is_anchored && !cursor->is_temp_anchored) {
        mark_clone(cursor->mark, &(cursor->anchor));
        if (use_srules) {
            cursor->sel_rule = srule_new_range(cursor->mark, cursor->anchor, 0, TB_REVERSE);
            buffer_add_srule(cursor->bview->buffer, cursor->sel_rule);
        }
        cursor->is_anchored = 1;
    } else {
        if (use_srules && cursor->sel_rule) {
            buffer_remove_srule(cursor->bview->buffer, cursor->sel_rule);
            srule_destroy(cursor->sel_rule);
            cursor->sel_rule = NULL;
        }
        mark_destroy(cursor->anchor);
        cursor->is_anchored = 0;
        cursor->is_temp_anchored = 0;
    }
    return MLE_OK;
}

// Drop cursor anchor
int cursor_drop_anchor(cursor_t *cursor, int use_srules) {
    if (cursor->is_anchored) return MLE_OK;
    return cursor_toggle_anchor(cursor, use_srules);
}

// Lift cursor anchor
int cursor_lift_anchor(cursor_t *cursor) {
    if (!cursor->is_anchored) return MLE_OK;
    return cursor_toggle_anchor(cursor, cursor->sel_rule ? 1 : 0);
}

// Get lo and hi marks in a is_anchored=1 cursor
int cursor_get_lo_hi(cursor_t *cursor, mark_t **ret_lo, mark_t **ret_hi) {
    if (!cursor->is_anchored) {
        return MLE_ERR;
    }
    if (mark_is_gt(cursor->anchor, cursor->mark)) {
        *ret_lo = cursor->mark;
        *ret_hi = cursor->anchor;
    } else {
        *ret_lo = cursor->anchor;
        *ret_hi = cursor->mark;
    }
    return MLE_OK;
}

// Get mark
int cursor_get_mark(cursor_t *cursor, mark_t **ret_mark) {
    *ret_mark = cursor->mark;
    return MLE_OK;
}

// Get anchor if anchored
int cursor_get_anchor(cursor_t *cursor, mark_t **ret_anchor) {
    *ret_anchor = cursor->is_anchored ? cursor->anchor : NULL;
    return MLE_OK;
}

// Make selection by strat
int cursor_select_by(cursor_t *cursor, const char *strat, int use_srules) {
    if (cursor->is_anchored) {
        return MLE_ERR;
    }
    if (strcmp(strat, "bracket") == 0) {
        return cursor_select_by_bracket(cursor, use_srules);
    } else if (strcmp(strat, "word") == 0) {
        return cursor_select_by_word(cursor, use_srules);
    } else if (strcmp(strat, "word_back") == 0) {
        return cursor_select_by_word_back(cursor, use_srules);
    } else if (strcmp(strat, "word_forward") == 0) {
        return cursor_select_by_word_forward(cursor, use_srules);
    } else if (strcmp(strat, "eol") == 0) {
        cursor_toggle_anchor(cursor, use_srules);
        mark_move_eol(cursor->anchor);
    } else if (strcmp(strat, "bol") == 0) {
        cursor_toggle_anchor(cursor, use_srules);
        mark_move_bol(cursor->anchor);
    } else if (strcmp(strat, "string") == 0) {
        return cursor_select_by_string(cursor, use_srules);
    } else if (strcmp(strat, "all") == 0) {
        mark_move_beginning(cursor->mark);
        cursor_toggle_anchor(cursor, use_srules);
        mark_move_end(cursor->mark);
    } else {
        MLE_RETURN_ERR(cursor->bview->editor, "Unrecognized cursor_select_by strat '%s'", strat);
    }
    return MLE_OK;
}

// Select by bracket
int cursor_select_by_bracket(cursor_t *cursor, int use_srules) {
    mark_t *orig;
    mark_clone(cursor->mark, &orig);
    if (mark_move_bracket_top(cursor->mark, MLE_BRACKET_PAIR_MAX_SEARCH) != MLBUF_OK) {
        mark_destroy(orig);
        return MLE_ERR;
    }
    cursor_toggle_anchor(cursor, use_srules);
    if (mark_move_bracket_pair(cursor->anchor, MLE_BRACKET_PAIR_MAX_SEARCH) != MLBUF_OK) {
        cursor_toggle_anchor(cursor, use_srules);
        mark_join(cursor->mark, orig);
        mark_destroy(orig);
        return MLE_ERR;
    }
    mark_move_by(cursor->mark, 1);
    mark_destroy(orig);
    return MLE_OK;
}

// Select by word-back
int cursor_select_by_word_back(cursor_t *cursor, int use_srules) {
    if (mark_is_at_word_bound(cursor->mark, -1)) return MLE_ERR;
    cursor_toggle_anchor(cursor, use_srules);
    mark_move_prev_re(cursor->mark, MLE_RE_WORD_BACK, sizeof(MLE_RE_WORD_BACK)-1);
    return MLE_OK;
}

// Select by word-forward
int cursor_select_by_word_forward(cursor_t *cursor, int use_srules) {
    if (mark_is_at_word_bound(cursor->mark, 1)) return MLE_ERR;
    cursor_toggle_anchor(cursor, use_srules);
    mark_move_next_re(cursor->mark, MLE_RE_WORD_FORWARD, sizeof(MLE_RE_WORD_FORWARD)-1);
    return MLE_OK;
}

// Select by string
int cursor_select_by_string(cursor_t *cursor, int use_srules) {
    mark_t *orig;
    uint32_t qchar;
    char *qre1 = "(?<!\\\\)[`'\"]";
    char qre2[16];
    int rv, left_right = 0;

    mark_clone(cursor->mark, &orig);

    for (left_right = 0; left_right <= 1; left_right++) {
        mark_join(cursor->mark, orig);

        // left_right==0 initially look left for quote
        // left_right==1 initially look right for quote
        rv = (left_right == 0 ? mark_move_prev_re : mark_move_next_re)
            (cursor->mark, qre1, strlen(qre1));
        if (rv != MLBUF_OK) continue;

        // Get quote char and make qre2
        mark_get_char_after(cursor->mark, &qchar);
        snprintf(qre2, sizeof(qre2), "(?<!\\\\)%c", (char)qchar);

        // Drop anchor
        if (left_right == 0) mark_move_by(cursor->mark, 1);
        cursor_drop_anchor(cursor, use_srules);

        // left_right==0 look right for quote pair
        // left_right==1 look left for quote pair
        rv = (left_right == 0 ? mark_move_next_re : mark_move_prev_re)
            (cursor->anchor, qre2, strlen(qre2));
        if (rv != MLBUF_OK) continue;

        // Selected!
        if (left_right == 1) mark_move_by(cursor->anchor, 1);
        mark_destroy(orig);
        return MLE_OK;
    }

    cursor_lift_anchor(cursor);
    mark_join(cursor->mark, orig);
    mark_destroy(orig);
    return MLE_ERR;
}

// Select by word
int cursor_select_by_word(cursor_t *cursor, int use_srules) {
    uint32_t after;
    if (mark_is_at_eol(cursor->mark)) return MLE_ERR;
    mark_get_char_after(cursor->mark, &after);
    if (!isalnum((char)after) && (char)after != '_') return MLE_ERR;
    if (!mark_is_at_word_bound(cursor->mark, -1)) {
        mark_move_prev_re(cursor->mark, MLE_RE_WORD_BACK, sizeof(MLE_RE_WORD_BACK)-1);
    }
    cursor_toggle_anchor(cursor, use_srules);
    mark_move_next_re(cursor->mark, MLE_RE_WORD_FORWARD, sizeof(MLE_RE_WORD_FORWARD)-1);
    return MLE_OK;
}

// Cut or copy text
int cursor_cut_copy(cursor_t *cursor, int is_cut, int use_srules, int append) {
    char *cutbuf;
    bint_t cutbuf_len;
    bint_t cur_len;
    if (!append && cursor->cut_buffer) {
        free(cursor->cut_buffer);
        cursor->cut_buffer = NULL;
    }
    if (!cursor->is_anchored) {
        use_srules = 0;
        cursor_toggle_anchor(cursor, use_srules);
        mark_move_bol(cursor->mark);
        mark_move_eol(cursor->anchor);
        if (!cursor->is_block) mark_move_by(cursor->anchor, 1);
    }
    if (cursor->is_block) {
        mark_block_get_between(cursor->mark, cursor->anchor, &cutbuf, &cutbuf_len);
    } else {
        mark_get_between(cursor->mark, cursor->anchor, &cutbuf, &cutbuf_len);
    }
    if (append && cursor->cut_buffer) {
        cur_len = strlen(cursor->cut_buffer);
        cursor->cut_buffer = realloc(cursor->cut_buffer, cur_len + cutbuf_len + 1);
        strncat(cursor->cut_buffer, cutbuf, cutbuf_len);
        free(cutbuf);
    } else {
        cursor->cut_buffer = cutbuf;
    }
    if (cursor->bview->editor->cut_buffer) free(cursor->bview->editor->cut_buffer);
    cursor->bview->editor->cut_buffer = strdup(cursor->cut_buffer);
    if (is_cut) {
        if (cursor->is_block) {
            mark_block_delete_between(cursor->mark, cursor->anchor);
        } else {
            mark_delete_between(cursor->mark, cursor->anchor);
        }
    }
    cursor_toggle_anchor(cursor, use_srules);
    return MLE_OK;
}

// Uncut from cursor cut_buffer
int cursor_uncut(cursor_t *cursor) {
    return cursor_uncut_ex(cursor, 0);
}

// Uncut editor-wide cut_buffer
int cursor_uncut_last(cursor_t *cursor) {
    return cursor_uncut_ex(cursor, 1);
}

// Regex search and replace
int cursor_replace(cursor_t *cursor, int interactive, char *opt_regex, char *opt_replacement) {
    char *regex;
    char *replacement;
    int wrapped;
    int all;
    char *yn;
    mark_t *lo_mark;
    mark_t *hi_mark;
    mark_t *orig_mark;
    mark_t *search_mark;
    mark_t *search_mark_end;
    int anchored_before;
    srule_t *highlight;
    bline_t *bline;
    bint_t col;
    bint_t char_count;
    bint_t orig_viewport_y;
    int pcre_rc;
    PCRE2_SIZE pcre_ovector[30];
    str_t repl_backref = {0};
    int num_replacements;

    if (!interactive && (!opt_regex || !opt_replacement)) {
        return MLE_ERR;
    }

    regex = NULL;
    replacement = NULL;
    wrapped = 0;
    lo_mark = NULL;
    hi_mark = NULL;
    orig_mark = NULL;
    search_mark = NULL;
    search_mark_end = NULL;
    anchored_before = 0;
    all = interactive ? 0 : 1;
    num_replacements = 0;
    mark_set_pcre_capture(&pcre_rc, pcre_ovector, 30);
    orig_viewport_y = -1;

    do {
        if (!interactive) {
            regex = strdup(opt_regex);
            replacement = strdup(opt_replacement);
        } else {
            editor_prompt(cursor->bview->editor, "replace: Search regex?", NULL, &regex);
            if (!regex) break;
            editor_prompt(cursor->bview->editor, "replace: Replacement string?", NULL, &replacement);
            if (!replacement) break;
        }
        orig_mark = buffer_add_mark(cursor->bview->buffer, NULL, 0);
        lo_mark = buffer_add_mark(cursor->bview->buffer, NULL, 0);
        hi_mark = buffer_add_mark(cursor->bview->buffer, NULL, 0);
        search_mark = buffer_add_mark(cursor->bview->buffer, NULL, 0);
        search_mark_end = buffer_add_mark(cursor->bview->buffer, NULL, 0);
        mark_join(search_mark, cursor->mark);
        mark_join(orig_mark, cursor->mark);
        orig_viewport_y = cursor->bview->viewport_y;
        orig_mark->lefty = 1; // lefty==1 avoids moving when text is inserted at mark
        lo_mark->lefty = 1;
        if (cursor->is_anchored) {
            anchored_before = mark_is_gt(cursor->mark, cursor->anchor);
            mark_join(lo_mark, !anchored_before ? cursor->mark : cursor->anchor);
            mark_join(hi_mark, anchored_before ? cursor->mark : cursor->anchor);
        } else {
            mark_move_beginning(lo_mark);
            mark_move_end(hi_mark);
        }
        while (1) {
            pcre_rc = 0;
            if (mark_find_next_re(search_mark, regex, strlen(regex), &bline, &col, &char_count) == MLBUF_OK
                && (mark_move_to(search_mark, bline->line_index, col) == MLBUF_OK)
                && (mark_is_gte(search_mark, lo_mark))
                && (mark_is_lt(search_mark, hi_mark))
                && (!wrapped || mark_is_lt(search_mark, orig_mark))
            ) {
                mark_move_to(search_mark_end, bline->line_index, col + char_count);
                mark_join(cursor->mark, search_mark);
                yn = NULL;
                if (all) {
                    yn = MLE_PROMPT_YES;
                } else if (interactive) {
                    highlight = srule_new_range(search_mark, search_mark_end, 0, TB_REVERSE);
                    buffer_add_srule(cursor->bview->buffer, highlight);
                    bview_rectify_viewport(cursor->bview);
                    bview_draw(cursor->bview);
                    editor_prompt(cursor->bview->editor, "replace: OK to replace? (y=yes, n=no, a=all, C-c=stop)",
                        &(editor_prompt_params_t) { .kmap = cursor->bview->editor->kmap_prompt_yna }, &yn
                    );
                    buffer_remove_srule(cursor->bview->buffer, highlight);
                    srule_destroy(highlight);
                    bview_draw(cursor->bview);
                }
                if (!yn) {
                    break;
                } else if (0 == strcmp(yn, MLE_PROMPT_YES) || 0 == strcmp(yn, MLE_PROMPT_ALL)) {
                    str_append_replace_with_backrefs(&repl_backref, search_mark->bline->data, replacement, pcre_rc, pcre_ovector, 30);
                    mark_replace_between(search_mark, search_mark_end, repl_backref.data, repl_backref.len);
                    str_free(&repl_backref);
                    num_replacements += 1;
                    if (0 == strcmp(yn, MLE_PROMPT_ALL)) all = 1;
                } else {
                    mark_move_by(search_mark, 1);
                }
            } else if (!wrapped) {
                mark_join(search_mark, lo_mark);
                wrapped = 1;
            } else {
                break;
            }
        }
    } while(0);

    if (cursor->is_anchored && lo_mark && hi_mark) {
        mark_join(cursor->mark, anchored_before ? hi_mark : lo_mark);
        mark_join(cursor->anchor, anchored_before ? lo_mark : hi_mark);
    } else if (orig_mark) {
        mark_join(cursor->mark, orig_mark);
    }

    mark_set_pcre_capture(NULL, NULL, 0);
    if (regex) free(regex);
    if (replacement) free(replacement);
    if (lo_mark) mark_destroy(lo_mark);
    if (hi_mark) mark_destroy(hi_mark);
    if (orig_mark) mark_destroy(orig_mark);
    if (search_mark) mark_destroy(search_mark);
    if (search_mark_end) mark_destroy(search_mark_end);

    if (interactive) {
        MLE_SET_INFO(cursor->bview->editor, "replace: Replaced %d instance(s)", num_replacements);
        if (orig_viewport_y >= 0) {
            bview_set_viewport_y(cursor->bview, orig_viewport_y, 1);
        } else {
            bview_rectify_viewport(cursor->bview);
        }
        bview_draw(cursor->bview);
    }

    return MLE_OK;
}

// Uncut (paste) text
static int cursor_uncut_ex(cursor_t *cursor, int editor_wide) {
    char *cut_buffer;
    cut_buffer = cursor->cut_buffer && !editor_wide ? cursor->cut_buffer : cursor->bview->editor->cut_buffer;
    if (!cut_buffer) return MLE_ERR;
    if (cursor->is_block) {
        mark_block_insert_before(cursor->mark, cut_buffer, strlen(cut_buffer));
    } else {
        mark_insert_before(cursor->mark, cut_buffer, strlen(cut_buffer));
    }
    return MLE_OK;
}
