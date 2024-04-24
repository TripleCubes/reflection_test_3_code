#include "err_msg.h"

#include "../tree/types.h"
#include <iostream>
#include <string>

namespace {
void str_grouped_token(std::string &result, const Branch &token) {
	if (token.str != "") {
		result += token.str;
	} else {
		for (int i = 0; i < (int)token.branch_list.size(); i++) {
			Branch v = token.branch_list[i];
			result += v.str;
		}
	}
}

void spaces(std::string &str, int n) {
	for (int i = 0; i < n; i++) {
		str += " ";
	}
}

void draw_err_pos(const Branch &token_list, int pos) {
	std::string str;
	std::string str_code;
	std::string str_arrow;
	int line = token_list.branch_list[pos].line;
	int column = token_list.branch_list[pos].column;
	str += "  " + std::to_string(line) + "  | ";
	int x = 0;

	for (int i = 0; i < (int)token_list.branch_list.size(); i++) {
		const Branch &v = token_list.branch_list[i];
		if (v.line == line) {
			if (v.column == column) {
				x = (int)str_code.length();
			}
			str_grouped_token(str_code, v);
			str_code += " ";
		}
		if (v.line > line) {
			break;
		}
	}

	spaces(str_arrow, (int)str.length() - 2);
	str_arrow += "| ";
	str += str_code;
	spaces(str_arrow, x);
	str_arrow += "^";

	std::cout << str << std::endl;
	std::cout << str_arrow << std::endl << std::endl;
}
}

void err_msg(const Branch &token_list, ErrMsgType type, int pos) {
	std::string s;
	switch (type) {
	case EXPECT_VALUE_AT_START:
		s = " value expected";
		break;
	case EXPECT_OP:
		s = " operator expected";
		break;
	case EXPECT_VALUE:
		s = " value expected";
		break;
	case EXPECT_CLOSE_ROUND_BRACKET:
		s = " ')' expected";
		break;
	case EXPECT_CLOSE_SQUARE_BRACKET:
		s = " ']' expected";
		break;
	case UNEXPECTED_CLOSE_ROUND_BRACKET:
		s = " ')' not expected";
		break;
	
	case CANT_PARSE:
		s = " cant parse";
		break;
	case EXPECT_VAR_TYPE:
		s = " var type expected";
		break;
	case EXPECT_RETURN_TYPE:
		s = " return type expected";
		break;
	
	case EXPECT_COLON:
		s = " ':' expected";
		break;
	case EXPECT_COMMA:
		s = " ',' expected";
		break;
	case EXPECT_ARROW:
		s = " '->' expected";
		break;
	case EXPECT_VAR_NAME:
		s = " var name expected";
		break;

		default:
		s = " unhandled";
		break;
	}

	const Branch &token = token_list.branch_list[pos];
	std::cout << token.line << ":" << token.column
		<< s << std::endl;
	draw_err_pos(token_list, pos);
	exit(0);
}
