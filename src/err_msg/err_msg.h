#ifndef ERR_MSG_ERR_MSG_H
#define ERR_MSG_ERR_MSG_H

struct Branch;

enum ErrMsgType: int {
	EXPECT_VALUE_AT_START,
	EXPECT_OP,
	EXPECT_VALUE,
	EXPECT_CLOSE_ROUND_BRACKET,
	EXPECT_CLOSE_SQUARE_BRACKET,
	UNEXPECTED_CLOSE_ROUND_BRACKET,

	CANT_PARSE,
	EXPECT_VAR_TYPE,
	EXPECT_RETURN_TYPE,

	EXPECT_COLON,
	EXPECT_COMMA,
	EXPECT_ARROW,
	EXPECT_VAR_NAME,
};

enum TypeErrMsgType: int {
	INCOMPATIBLE_OP,
	INCOMPATIBLE_ARGV_NUM,
	INCOMPATIBLE_TYPE,
};

void err_msg(const Branch &token_list, ErrMsgType type, int pos);
void type_err_msg(const Branch &branch, TypeErrMsgType type);

#endif
