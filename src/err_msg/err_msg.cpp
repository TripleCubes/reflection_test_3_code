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

void draw_err_pos(const Branch &token_list, int pos, int offset) {
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
	spaces(str_arrow, x + offset);
	str_arrow += "^";

	std::cout << str << std::endl;
	std::cout << str_arrow << std::endl << std::endl;
}
}

void err_right_side_start(const Branch &token_list, int pos) {
	const Branch &token = token_list.branch_list[pos];
	std::cout << token.line << ":" << token.column
		<< " expected a value" << std::endl;
	draw_err_pos(token_list, pos, 0);
	exit(0);
}

void err_right_side_expect_op(const Branch &token_list, int pos) {
	const Branch &token = token_list.branch_list[pos];
	std::cout << token.line << ":" << token.column + 1
		<< " expected an operator" << std::endl;
	draw_err_pos(token_list, pos, 1);
	exit(0);
}

void err_right_side_expect_value(const Branch &token_list, int pos) {
	const Branch &token = token_list.branch_list[pos];
	std::cout << token.line << ":" << token.column + 1
		<< " expected a value" << std::endl;
	draw_err_pos(token_list, pos, 1);
	exit(0);
}

void err_right_side_unexpected_close_bracket(const Branch &token_list,
                                             int pos) {
	const Branch &token = token_list.branch_list[pos];
	std::cout << token.line << ":" << token.column
		<< " unexpected close round bracket" << std::endl;
	draw_err_pos(token_list, pos, 0);
	exit(0);
}

void err_cant_parse(const Branch &token_list, int pos) {
	const Branch &token = token_list.branch_list[pos];
	std::cout << token.line << ":" << token.column
		<< " cant parse" << std::endl;
	draw_err_pos(token_list, pos, 0);
	exit(0);
}

void err_not_a_var_type(const Branch &token_list, int pos) {
	const Branch &token = token_list.branch_list[pos];
	std::cout << token.line << ":" << token.column
		<< " expected a var type" << std::endl;
	draw_err_pos(token_list, pos, 0);
	exit(0);
}
