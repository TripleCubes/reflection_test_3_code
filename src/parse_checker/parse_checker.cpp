#include "parse_checker.h"

#include "../tree/types.h"
#include "../err_msg/err_msg.h"
#include "../tree/var_types.h"
#include <vector>
#include <string>
#include <iostream>

namespace {
int sz(int start, int end) {
	return end - start + 1;
}

bool is_value(const Branch &token) {
	if (token.type == NAME || token.type == NUM || token.type == STR
	|| token.str == "true" || token.str == "false") {
		return true;
	}
	return false;
}

bool is_reverse_op(const Branch &token) {
	if (token.type != OPERATOR) {
		return false;
	}
	if (token.str != "-" && token.str != "not") {
		return false;
	}
	return true;
}

std::vector<std::string> op_list = {
	"+", "-", "*", "/", "//", "%", "..", "not", "and", "or",
	"==", "~=", ">=", "<=", "<", ">",
};

bool is_in_list(const std::vector<std::string> &list,
const std::string &str) {
	for (int i = 0; i < (int)list.size(); i++) {
		if (str == list[i]) {
			return true;
		}
	}
	return false;
}

bool is_op(const Branch &token) {
	if (token.type != OPERATOR) {
		return false;
	}
	if (!is_in_list(op_list, token.str)) {
		return false;
	}
	return true;
}

bool is_next_to_value(const Branch &nx) {
	if (is_op(nx) || nx.str == "(" 
	|| nx.str == ")" || nx.type == NONE) {
		return true;
	}
	return false;
}

bool is_next_to_op(const Branch &nx, const Branch &nx_nx) {
	if (is_value(nx)
	|| (is_reverse_op(nx) && is_value(nx_nx))
	|| nx.str == "(") {
		return true;
	}
	return false;
}

bool is_next_to_open_bracket(const Branch &nx) {
	if (is_value(nx) || is_reverse_op(nx)) {
		return true;
	}
	return false;
}

bool is_next_to_close_bracket(const Branch &nx) {
	if (is_op(nx) || nx.type == NONE || nx.str == ")") {
		return true;
	}
	return false;
}

bool f_is_next_to_value(const Branch &nx) {
	if (is_next_to_value(nx) || nx.str == "," || nx.str == ")") {
		return true;
	}
	return false;
}

bool f_is_next_to_close_bracket(const Branch &nx, int f_bracket_count){
	if (is_next_to_close_bracket(nx)
	|| (nx.str == "," && f_bracket_count != 1)) {
		return true;
	}
	return false;
}

bool f_is_next_to_comma(const Branch &nx) {
	if (is_value(nx) || is_reverse_op(nx) || nx.str == "(") {
		return true;
	}
	return false;
}

void right_side_check(const Branch &token_list, int start, int end,
bool in_funccall) {
	int f_bracket_count = 0;
	int bracket_count = 0;
	for (int i = start; i <= end; i++) {
		Branch v = token_list.branch_list[i];
		Branch nx;
		if (i + 1 <= end) {
			nx = token_list.branch_list[i + 1];
		}
		Branch nx_nx;
		if (i + 2 <= end) {
			nx_nx = token_list.branch_list[i + 2];
		}

		if (i == start && (!is_value(v) && !is_reverse_op(v)
		&& v.str != "(")) {
			err_msg(token_list, EXPECT_VALUE_AT_START, start);
		}

		if (!in_funccall) {
			if (is_value(v) && !is_next_to_value(nx)) {
				err_msg(token_list, EXPECT_OP, i);
			}

			if (is_op(v) && !is_next_to_op(nx, nx_nx)) {
				err_msg(token_list, EXPECT_VALUE, i);
			}

			if (v.str == "(" && !is_next_to_open_bracket(nx)) {
				err_msg(token_list, EXPECT_VALUE, i);
			}

			if (v.str == ")" && !is_next_to_close_bracket(nx)) {
				err_msg(token_list, EXPECT_OP, i);
			}
		}

		if (in_funccall) {
			if (v.str == "(" && !is_next_to_open_bracket(nx)) {
				err_msg(token_list, EXPECT_VALUE, i);
			}
			
			if (is_value(v) && !f_is_next_to_value(nx)) {
				err_msg(token_list, EXPECT_OP, i);
			}

			if (is_op(v) && !is_next_to_op(nx, nx_nx)) {
				err_msg(token_list, EXPECT_VALUE, i);
			}

			if (v.str == ")"
			&& !f_is_next_to_close_bracket(nx, f_bracket_count)) {
				err_msg(token_list, EXPECT_OP, i);
			}

			if (v.str == "," && !f_is_next_to_comma(nx)) {
				err_msg(token_list, EXPECT_VALUE, i);
			}

			if (v.str == "(") {
				f_bracket_count++;
			}
			else if (v.str == ")") {
				f_bracket_count--;
				if (f_bracket_count == 0) {
					in_funccall = false;
				}
			}
		}

		if (is_value(v) && nx.str == "(") {
			in_funccall = true;
		}

		if (v.str == "(") {
			bracket_count++;
		}
		else if (v.str == ")") {
			bracket_count--;
			if (bracket_count < 0) {
				err_msg(token_list, UNEXPECTED_CLOSE_ROUND_BRACKET, i);
			}
		}
	}
	
	if (bracket_count > 0) {
		err_msg(token_list, EXPECT_CLOSE_ROUND_BRACKET, end);
	}
}

void varnew_check(const Branch &token_list, int start, int end) {
	int equal_sign_pos = start + 4;
	if (sz(start, end) < 6) {
		err_msg(token_list, CANT_PARSE, start);
	}
	if (token_list.branch_list[start + 1].type != NAME) {
		err_msg(token_list, CANT_PARSE, start + 1);
	}
	if (token_list.branch_list[start + 2].str != ":") {
		err_msg(token_list, CANT_PARSE, start + 2);
	}
	if (!is_var_type(token_list.branch_list[start + 3])) {
		err_msg(token_list, CANT_PARSE, start + 3);
	}
	if (token_list.branch_list[start + 4].str == "[") {
		equal_sign_pos = start + 6;
		if (sz(start, end) < 8) {
			err_msg(token_list, CANT_PARSE, start);
		}
		if (token_list.branch_list[start + 5].str != "]") {
			err_msg(token_list, CANT_PARSE, start);
		}
	}
	if (token_list.branch_list[equal_sign_pos].str != "=") {
		err_msg(token_list, CANT_PARSE, start);
	}

	right_side_check(token_list, equal_sign_pos + 1, end, false);
}

void assign_check(const Branch &token_list, int start, int end) {
	if (sz(start, end) < 3) {
		err_msg(token_list, CANT_PARSE, start);
	}

	if (token_list.branch_list[start].type != NAME) {
		err_msg(token_list, CANT_PARSE, start);
	}

	if (token_list.branch_list[start + 1].str != "=") {
		err_msg(token_list, CANT_PARSE, start + 1);
	}

	right_side_check(token_list, start + 2, end, false);
}

void funccall_check(const Branch &token_list, int start, int end) {
	if (sz(start, end) < 3) {
		err_msg(token_list, CANT_PARSE, start);
	}

	if (token_list.branch_list[start].type != NAME) {
		err_msg(token_list, CANT_PARSE, start);
	}

	if (token_list.branch_list[start + 1].str != "(") {
		err_msg(token_list, CANT_PARSE, start + 1);
	}

	if (token_list.branch_list[end].str != ")") {
		err_msg(token_list, CANT_PARSE, end);
	}

	right_side_check(token_list, start + 2, end - 1, true);
}

void argument_list_check(const Branch &token_list, int start, int end){
	for (int i = start; i <= end; i++) {
		const Branch &v = token_list.branch_list[i];
		Branch nx;
		if (i + 1 <= end) {
			nx = token_list.branch_list[i + 1];
		}
		Branch prev;
		if (i - 1 >= start) {
			prev = token_list.branch_list[i - 1];
		}

		if (v.type == NAME && nx.str != ":") {
			err_msg(token_list, EXPECT_COLON, i);
		}

		if (v.str == ":" && !is_var_type(nx)) {
			err_msg(token_list, EXPECT_VAR_TYPE, i);
		}

		if (is_var_type(v) && prev.str == ":" && !(nx.str == ","
		|| nx.str == "[" || nx.type == NONE)) {
			err_msg(token_list, EXPECT_COMMA, i);
		}

		if (v.str == "[" && nx.str != "]") {
			err_msg(token_list, EXPECT_CLOSE_SQUARE_BRACKET, i);
		}

		if (v.str == "]" && !(nx.str == ","
		|| nx.type == NONE)) {
			err_msg(token_list, EXPECT_COMMA, i);
		}

		if (v.str == "," && nx.type != NAME) {
			err_msg(token_list, EXPECT_VAR_NAME, i);
		}
	}
}

void funcnew_check(const Branch &token_list, int start, int end) {
	if (sz(start, end) < 7) {
		err_msg(token_list, CANT_PARSE, start);
	}

	if (token_list.branch_list[start + 1].type != NAME) {
		err_msg(token_list, CANT_PARSE, start + 1);
	}

	if (token_list.branch_list[start + 2].str != "(") {
		err_msg(token_list, CANT_PARSE, start + 2);
	}

	int bracket_close = 0;
	for (int i = start + 3; i <= end; i++) {
		if (i == end) {
			err_msg(token_list, CANT_PARSE, start);
		}

		const Branch &v = token_list.branch_list[i];
		if (v.str == ")") {
			bracket_close = i;
			break;
		}
	}

	argument_list_check(token_list, start + 3, bracket_close - 1);

	if (end - start < bracket_close - start + 3) {
		err_msg(token_list, CANT_PARSE, start);
	}

	if (token_list.branch_list[bracket_close + 1].str != "->") {
		err_msg(token_list, EXPECT_ARROW, bracket_close + 1);
	}

	if (!is_return_type(token_list.branch_list[bracket_close + 2])) {
		err_msg(token_list, EXPECT_RETURN_TYPE, bracket_close + 1);
	}
}

void if_check(const Branch &token_list, int start, int end) {
	if (sz(start, end) < 4) {
		err_msg(token_list, CANT_PARSE, start);
	}

	int then_pos = 0;
	for (int i = start + 1; i <= end; i++) {
		if (i == end) {
			err_msg(token_list, CANT_PARSE, start);
		}

		const Branch &v = token_list.branch_list[i];
		if (v.str == "then") {
			then_pos = i;
			break;
		}
	}

	right_side_check(token_list, start + 1, then_pos - 1, false);
}
}

void parse_check(const Branch &token_list, BranchType branch_type,
int start, int end) {
	if (branch_type == VARNEW) {
		varnew_check(token_list, start, end);
	}
	else if (branch_type == ASSIGN) {
		assign_check(token_list, start, end);
	}
	else if (branch_type == FUNCCALL) {
		funccall_check(token_list, start, end);
	}
	else if (branch_type == FUNCNEW) {
		funcnew_check(token_list, start, end);
	}
	else if (branch_type == IF) {
		if_check(token_list, start, end);
	}
	else if (branch_type == ELSEIF) {
		if_check(token_list, start, end);
	}
}
