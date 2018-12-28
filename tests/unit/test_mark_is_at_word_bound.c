#include "test.h"

char* str = "obj->method() yes bob";

void test(buffer_t* buf, mark_t* cur) {
    mark_move_beginning(cur); // |obj->method() yes bob
    ASSERT("l_bol", 1, mark_is_at_word_bound(cur, -1));
    ASSERT("x_bol", 1, mark_is_at_word_bound(cur, 0));
    ASSERT("r_bol", 0, mark_is_at_word_bound(cur, 1));

    mark_move_by(cur, 1);     // o|bj->method() yes bob
    ASSERT("l_mid", 0, mark_is_at_word_bound(cur, -1));
    ASSERT("x_mid", 0, mark_is_at_word_bound(cur, 0));
    ASSERT("r_mid", 0, mark_is_at_word_bound(cur, 1));

    mark_move_by(cur, 2);     // obj|->method() yes bob
    ASSERT("l_eow", 0, mark_is_at_word_bound(cur, -1));
    ASSERT("x_eow", 1, mark_is_at_word_bound(cur, 0));
    ASSERT("r_eow", 1, mark_is_at_word_bound(cur, 1));

    mark_move_by(cur, 1);     // obj-|>method() yes bob
    ASSERT("l_sym", 0, mark_is_at_word_bound(cur, -1));
    ASSERT("x_sym", 0, mark_is_at_word_bound(cur, 0));
    ASSERT("r_sym", 0, mark_is_at_word_bound(cur, 1));

    mark_move_by(cur, 8);     // obj->method(|) yes bob
    ASSERT("l_sym2", 0, mark_is_at_word_bound(cur, -1));
    ASSERT("x_sym2", 0, mark_is_at_word_bound(cur, 0));
    ASSERT("r_sym2", 0, mark_is_at_word_bound(cur, 1));

    mark_move_by(cur, 1);     // obj->method()| yes bob
    ASSERT("l_spa1", 0, mark_is_at_word_bound(cur, -1));
    ASSERT("x_spa1", 0, mark_is_at_word_bound(cur, 0));
    ASSERT("r_spa1", 0, mark_is_at_word_bound(cur, 1));

    mark_move_by(cur, 1);     // obj->method() |yes bob
    ASSERT("l_sow", 1, mark_is_at_word_bound(cur, -1));
    ASSERT("x_sow", 1, mark_is_at_word_bound(cur, 0));
    ASSERT("r_sow", 0, mark_is_at_word_bound(cur, 1));

    mark_move_by(cur, 3);     // obj->method() yes| bob
    ASSERT("l_eow2", 0, mark_is_at_word_bound(cur, -1));
    ASSERT("x_eow2", 1, mark_is_at_word_bound(cur, 0));
    ASSERT("r_eow2", 1, mark_is_at_word_bound(cur, 1));

    mark_move_eol(cur);       // obj->method() yes bob|
    ASSERT("l_eow2", 0, mark_is_at_word_bound(cur, -1));
    ASSERT("x_eow2", 1, mark_is_at_word_bound(cur, 0));
    ASSERT("r_eow2", 1, mark_is_at_word_bound(cur, 1));
}
