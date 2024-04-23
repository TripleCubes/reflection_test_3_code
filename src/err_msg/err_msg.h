#ifndef ERR_MSG_ERR_MSG_H
#define ERR_MSG_ERR_MSG_H

struct Branch;

void err_right_side_start(const Branch &token_list, int pos);
void err_right_side_expect_op(const Branch &token_list, int pos);
void err_right_side_expect_value(const Branch &token_list, int pos);
void err_right_side_unexpected_close_bracket(const Branch &token_list,
                                             int pos);
void err_cant_parse(const Branch &token_list, int pos);
void err_not_a_var_type(const Branch &token_list, int pos);
void err_not_a_return_type(const Branch &token_list, int pos);

void err_expect_colon(const Branch &token_list, int pos);
void err_expect_comma(const Branch &token_list, int pos);
void err_expect_close_square_bracket(const Branch &token_list,int pos);
void err_expect_var_name(const Branch &token_list, int pos);

void err_expect_arrow(const Branch &token_list, int pos);

#endif
