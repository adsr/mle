#include "mle.h"

// Toggle cursor anchor
int cursor_toggle_anchor(cursor_t* cursor, int use_srules) {
    if (!cursor->is_anchored) {
        mark_clone(cursor->mark, &(cursor->anchor));
        if (use_srules) {
            cursor->sel_rule = srule_new_range(cursor->mark, cursor->anchor, 0, TB_REVERSE);
            buffer_add_srule(cursor->bview->buffer, cursor->sel_rule);
        }
        cursor->is_anchored = 1;
    } else {
        if (use_srules) {
            buffer_remove_srule(cursor->bview->buffer, cursor->sel_rule);
            srule_destroy(cursor->sel_rule);
            cursor->sel_rule = NULL;
        }
        mark_destroy(cursor->anchor);
        cursor->is_anchored = 0;
    }
    return MLE_OK;
}

// Drop cursor anchor
int cursor_drop_anchor(cursor_t* cursor) {
    if (cursor->is_anchored) return MLE_OK;
    return cursor_toggle_anchor(cursor, 1);
}

// Lift cursor anchor
int cursor_lift_anchor(cursor_t* cursor) {
    if (!cursor->is_anchored) return MLE_OK;
    return cursor_toggle_anchor(cursor, 1);
}

// Get lo and hi marks in a is_anchored=1 cursor
int cursor_get_lo_hi(cursor_t* cursor, mark_t** ret_lo, mark_t** ret_hi) {
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

// Make selection by strat
int cursor_select_by(cursor_t* cursor, const char* strat) {
    if (cursor->is_anchored) {
        return MLE_ERR;
    }
    if (strcmp(strat, "bracket") == 0) {
        return cursor_select_by_bracket(cursor);
    } else if (strcmp(strat, "word") == 0) {
        return cursor_select_by_word(cursor);
    } else if (strcmp(strat, "word_back") == 0) {
        return cursor_select_by_word_back(cursor);
    } else if (strcmp(strat, "word_forward") == 0) {
        return cursor_select_by_word_forward(cursor);
    } else if (strcmp(strat, "eol") == 0 && !mark_is_at_eol(cursor->mark)) {
        cursor_toggle_anchor(cursor, 0);
        mark_move_eol(cursor->anchor);
    } else if (strcmp(strat, "bol") == 0 && !mark_is_at_bol(cursor->mark)) {
        cursor_toggle_anchor(cursor, 0);
        mark_move_bol(cursor->anchor);
    } else if (strcmp(strat, "string") == 0) {
        return cursor_select_by_string(cursor);
    } else {
        MLE_RETURN_ERR(cursor->bview->editor, "Unrecognized cursor_select_by strat '%s'", strat);
    }
    return MLE_OK;
}

// Select by bracket
int cursor_select_by_bracket(cursor_t* cursor) {
    mark_t* orig;
    mark_clone(cursor->mark, &orig);
    if (mark_move_bracket_top(cursor->mark, MLE_BRACKET_PAIR_MAX_SEARCH) != MLBUF_OK) {
        mark_destroy(orig);
        return MLE_ERR;
    }
    cursor_toggle_anchor(cursor, 0);
    if (mark_move_bracket_pair(cursor->anchor, MLE_BRACKET_PAIR_MAX_SEARCH) != MLBUF_OK) {
        cursor_toggle_anchor(cursor, 0);
        mark_join(cursor->mark, orig);
        mark_destroy(orig);
        return MLE_ERR;
    }
    mark_move_by(cursor->mark, 1);
    mark_destroy(orig);
    return MLE_OK;
}

// Select by word-back
int cursor_select_by_word_back(cursor_t* cursor) {
    if (mark_is_at_word_bound(cursor->mark, -1)) return MLE_ERR;
    cursor_toggle_anchor(cursor, 0);
    mark_move_prev_re(cursor->mark, MLE_RE_WORD_BACK, sizeof(MLE_RE_WORD_BACK)-1);
    return MLE_OK;
}

// Select by word-forward
int cursor_select_by_word_forward(cursor_t* cursor) {
    if (mark_is_at_word_bound(cursor->mark, 1)) return MLE_ERR;
    cursor_toggle_anchor(cursor, 0);
    mark_move_next_re(cursor->mark, MLE_RE_WORD_FORWARD, sizeof(MLE_RE_WORD_FORWARD)-1);
    return MLE_OK;
}

// Select by string
int cursor_select_by_string(cursor_t* cursor) {
    mark_t* orig;
    uint32_t qchar;
    char* qre;
    mark_clone(cursor->mark, &orig);
    if (mark_move_prev_re(cursor->mark, "(?<!\\\\)[`'\"]", strlen("(?<!\\\\)[`'\"]")) != MLBUF_OK) {
        mark_destroy(orig);
        return MLE_ERR;
    }
    cursor_toggle_anchor(cursor, 0);
    mark_get_char_after(cursor->mark, &qchar);
    mark_move_by(cursor->mark, 1);
    if (qchar == '"') {
        qre = "(?<!\\\\)\"";
    } else if (qchar == '\'') {
        qre = "(?<!\\\\)'";
    } else {
        qre = "(?<!\\\\)`";
    }
    if (mark_move_next_re(cursor->anchor, qre, strlen(qre)) != MLBUF_OK) {
        cursor_toggle_anchor(cursor, 0);
        mark_join(cursor->mark, orig);
        mark_destroy(orig);
        return MLE_ERR;
    }
    mark_destroy(orig);
    return MLE_OK;
}

// Select by word
int cursor_select_by_word(cursor_t* cursor) {
    uint32_t after;
    if (mark_is_at_eol(cursor->mark)) return MLE_ERR;
    mark_get_char_after(cursor->mark, &after);
    if (!isalnum((char)after) && (char)after != '_') return MLE_ERR;
    if (!mark_is_at_word_bound(cursor->mark, -1)) {
        mark_move_prev_re(cursor->mark, MLE_RE_WORD_BACK, sizeof(MLE_RE_WORD_BACK)-1);
    }
    cursor_toggle_anchor(cursor, 0);
    mark_move_next_re(cursor->mark, MLE_RE_WORD_FORWARD, sizeof(MLE_RE_WORD_FORWARD)-1);
    return MLE_OK;
}
