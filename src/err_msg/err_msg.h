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
	EXPECT_CLOSE_CURLY_BRACKET,
	UNEXPECTED_CLOSE_ROUND_BRACKET,
	OPEN_CURLY_IN_ASSIGN,

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
	TYPE_NOT_DECLARED,
	VAR_ALREADY_DECLARED,
	FUNC_ALREADY_DECLARED,
	TYPE_ALREADY_DECLARED,

	INCOMPATIBLE_INHERITANCE_TYPE,
};

void err_msg(const Branch &branch, ErrMsgType type);
void type_err_msg(const Branch &branch, TypeErrMsgType type,
	const std::string &type1, const std::string &type2);
void err_msg_set_code_str(const std::string &in_code_str);
std::string get_type_err_msg_str();

#endif
