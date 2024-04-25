#ifndef ERR_MSG_ERR_MSG_H
#define ERR_MSG_ERR_MSG_H

#include <string>

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

void err_msg(const Branch &branch, ErrMsgType type);
void type_err_msg(const Branch &branch, TypeErrMsgType type,
	const std::string &type1, const std::string &type2);
void err_msg_set_code_str(const std::string &in_code_str);
std::string get_type_err_msg_str();

#endif
