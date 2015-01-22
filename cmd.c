#include "mle.h"

int cmd_insert_data(cmd_context_t* ctx) {
    char data[6];
    size_t data_len;
    data_len = utf8_unicode_to_char(data, ctx->input.ch);
    return mark_insert_after(ctx->cursor->mark, data, data_len); // TODO mult cursors
}

int cmd_insert_tab(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_delete_before(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_delete_after(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_move_bol(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_move_eol(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_move_beginning(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_move_end(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_move_left(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_move_right(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_move_up(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_move_down(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_move_page_up(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_move_page_down(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_move_to_line(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_move_word_forward(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_move_word_back(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_anchor_sel_bound(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_search(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_search_next(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_replace(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_isearch(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_delete_word_before(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_delete_word_after(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_cut(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_uncut(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_save(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_open(cmd_context_t* ctx) {
return MLE_OK; }
int cmd_quit(cmd_context_t* ctx) {
return MLE_OK; }
