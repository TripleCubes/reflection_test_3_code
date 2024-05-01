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
	|| (is_reverse_op(nx) && (is_value(nx_nx) || nx_nx.str == "("))
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
			err_msg(v, EXPECT_VALUE_AT_START);
		}

		if (!in_funccall) {
			if (is_value(v) && !is_next_to_value(nx)) {
				err_msg(v, EXPECT_OP);
			}

			if (is_op(v) && !is_next_to_op(nx, nx_nx)) {
				err_msg(v, EXPECT_VALUE);
			}

			if (v.str == "(" && !is_next_to_open_bracket(nx)) {
				err_msg(v, EXPECT_VALUE);
			}

			if (v.str == ")" && !is_next_to_close_bracket(nx)) {
				err_msg(v, EXPECT_OP);
			}
		}

		if (in_funccall) {
			if (v.str == "(" && !is_next_to_open_bracket(nx)) {
				err_msg(v, EXPECT_VALUE);
			}
			
			if (is_value(v) && !f_is_next_to_value(nx)) {
				err_msg(v, EXPECT_OP);
			}

			if (is_op(v) && !is_next_to_op(nx, nx_nx)) {
				err_msg(v, EXPECT_VALUE);
			}

			if (v.str == ")"
			&& !f_is_next_to_close_bracket(nx, f_bracket_count)) {
				err_msg(v, EXPECT_OP);
			}

			if (v.str == "," && !f_is_next_to_comma(nx)) {
				err_msg(v, EXPECT_VALUE);
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
				err_msg(v, UNEXPECTED_CLOSE_ROUND_BRACKET);
			}
		}
	}
	
	const Branch &end_token = token_list.branch_list[end];
	if (bracket_count > 0) {
		err_msg(end_token, EXPECT_CLOSE_ROUND_BRACKET);
	}
}

void varnew_check(const Branch &token_list, int start, int end) {
	int equal_sign_pos = start + 4;
	if (sz(start, end) < 6) {
		err_msg(token_list.branch_list[start], CANT_PARSE);
	}
	if (token_list.branch_list[start + 1].type != NAME) {
		err_msg(token_list.branch_list[start + 1], CANT_PARSE);
	}
	if (token_list.branch_list[start + 2].str != ":") {
		err_msg(token_list.branch_list[start + 2], CANT_PARSE);
	}
	if (!is_var_type(token_list.branch_list[start + 3])) {
		err_msg(token_list.branch_list[start + 3], CANT_PARSE);
	}
	if (token_list.branch_list[start + 4].str == "[") {
		equal_sign_pos = start + 6;
		if (sz(start, end) < 8) {
			err_msg(token_list.branch_list[start], CANT_PARSE);
		}
		if (token_list.branch_list[start + 5].str != "]") {
			err_msg(token_list.branch_list[start], CANT_PARSE);
		}
	}
	if (token_list.branch_list[equal_sign_pos].str != "=") {
		err_msg(token_list.branch_list[start], CANT_PARSE);
	}
	
	if (token_list.branch_list[equal_sign_pos + 1].str == "{") {
		if (token_list.branch_list[end].str != "}"
		|| end != equal_sign_pos + 2) {
			err_msg(token_list.branch_list[equal_sign_pos + 1],
					CANT_PARSE);
		}
		else {
			return;
		}
	}

	if (token_list.branch_list[equal_sign_pos + 1].str == "[") {
		if (token_list.branch_list[end].str != "]") {
			err_msg(token_list.branch_list[end],
			        EXPECT_CLOSE_SQUARE_BRACKET);
		}
		else {
			right_side_check(token_list,equal_sign_pos+2,end-1,true);
			return;
		}
	}

	right_side_check(token_list, equal_sign_pos + 1, end, false);
}

void assign_check(const Branch &token_list, int start, int end) {
	if (sz(start, end) < 3) {
		err_msg(token_list.branch_list[start], CANT_PARSE);
	}

	if (token_list.branch_list[start].type != NAME) {
		err_msg(token_list.branch_list[start], CANT_PARSE);
	}

	if (token_list.branch_list[start + 1].str != "=") {
		err_msg(token_list.branch_list[start + 1], CANT_PARSE);
	}

	int equal_sign_pos = start + 1;
	
	if (token_list.branch_list[equal_sign_pos + 1].str == "{") {
		err_msg(token_list.branch_list[equal_sign_pos + 1],
		        OPEN_CURLY_IN_ASSIGN);
	}

	if (token_list.branch_list[equal_sign_pos + 1].str == "[") {
		if (token_list.branch_list[end].str != "]") {
			err_msg(token_list.branch_list[end],
			        EXPECT_CLOSE_SQUARE_BRACKET);
		}
		else {
			right_side_check(token_list,equal_sign_pos+2,end-1,true);
			return;
		}
	}

	right_side_check(token_list, equal_sign_pos + 1, end, false);
}

void funccall_check(const Branch &token_list, int start, int end) {
	if (sz(start, end) < 3) {
		err_msg(token_list.branch_list[start], CANT_PARSE);
	}

	if (token_list.branch_list[start].type != NAME) {
		err_msg(token_list.branch_list[start], CANT_PARSE);
	}

	if (token_list.branch_list[start + 1].str != "(") {
		err_msg(token_list.branch_list[start + 1], CANT_PARSE);
	}

	if (token_list.branch_list[end].str != ")") {
		err_msg(token_list.branch_list[end], CANT_PARSE);
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

		if (v.type == NAME && (prev.str == "," || i == 0)
		&& nx.str != ":") {
			err_msg(v, EXPECT_COLON);
		}

		if (v.str == ":" && !is_var_type(nx)) {
			err_msg(v, EXPECT_VAR_TYPE);
		}

		if (is_var_type(v) && prev.str == ":" && !(nx.str == ","
		|| nx.str == "[" || nx.type == NONE)) {
			err_msg(v, EXPECT_COMMA);
		}

		if (v.str == "[" && nx.str != "]") {
			err_msg(v, EXPECT_CLOSE_SQUARE_BRACKET);
		}

		if (v.str == "]" && !(nx.str == ","
		|| nx.type == NONE)) {
			err_msg(v, EXPECT_COMMA);
		}

		if (v.str == "," && nx.type != NAME) {
			err_msg(v, EXPECT_VAR_NAME);
		}
	}
}

void funcnew_check(const Branch &token_list, int start, int end) {
	if (sz(start, end) < 7) {
		err_msg(token_list.branch_list[start], CANT_PARSE);
	}

	if (token_list.branch_list[start + 1].type != NAME) {
		err_msg(token_list.branch_list[start + 1], CANT_PARSE);
	}

	if (token_list.branch_list[start + 2].str != "(") {
		err_msg(token_list.branch_list[start + 2], CANT_PARSE);
	}

	int bracket_close = 0;
	for (int i = start + 3; i <= end; i++) {
		if (i == end) {
			err_msg(token_list.branch_list[start], CANT_PARSE);
		}

		const Branch &v = token_list.branch_list[i];
		if (v.str == ")") {
			bracket_close = i;
			break;
		}
	}

	argument_list_check(token_list, start + 3, bracket_close - 1);

	if (end - start < bracket_close - start + 3) {
		err_msg(token_list.branch_list[start], CANT_PARSE);
	}

	if (token_list.branch_list[bracket_close + 1].str != "->") {
		err_msg(token_list.branch_list[bracket_close+1], EXPECT_ARROW);
	}

	if (!is_return_type(token_list.branch_list[bracket_close + 2])) {
		err_msg(token_list.branch_list[bracket_close+1],
		        EXPECT_RETURN_TYPE);
	}
}

void if_check(const Branch &token_list, int start, int end) {
	if (sz(start, end) < 4) {
		err_msg(token_list.branch_list[start], CANT_PARSE);
	}

	int then_pos = 0;
	for (int i = start + 1; i <= end; i++) {
		if (i == end) {
			err_msg(token_list.branch_list[start], CANT_PARSE);
		}

		const Branch &v = token_list.branch_list[i];
		if (v.str == "then") {
			then_pos = i;
			break;
		}
	}

	right_side_check(token_list, start + 1, then_pos - 1, false);
}

void elseif_check(const Branch &token_list, int start, int end) {
	if (sz(start, end) < 3) {
		err_msg(token_list.branch_list[start], CANT_PARSE);
	}

	for (int i = start + 1; i <= end; i++) {
		const Branch &v = token_list.branch_list[i];
		if (v.str == "then") {
			break;
		}

		if (i == end) {
			err_msg(token_list.branch_list[start], CANT_PARSE);
		}
	}
}

void for_check(const Branch &token_list, int start, int end) {
	const std::vector<Branch> &bl = token_list.branch_list;

	if (bl[start + 1].type != NAME) {
		err_msg(bl[start + 1], CANT_PARSE);
	}

	if (bl[start + 2].str != "=") {
		err_msg(bl[start + 2], CANT_PARSE);
	}

	int right_side_start = start + 3;
	int right_side_end = 0;
	for (int i = right_side_start; i <= end; i++) {
		if (i == end) {
			err_msg(bl[right_side_start], CANT_PARSE);
		}

		if (bl[i].str == "do") {
			right_side_end = i - 1;
			break;
		}
	}

	right_side_check(token_list,right_side_start,right_side_end,true);
}

void foreach_check(const Branch &token_list, int start, int end) {
	const std::vector<Branch> &bl = token_list.branch_list;

	if (bl[start + 1].type != NAME) {
		err_msg(bl[start + 1], CANT_PARSE);
	}

	if (bl[start + 2].str != ",") {
		err_msg(bl[start + 2], CANT_PARSE);
	}

	if (bl[start + 3].type != NAME) {
		err_msg(bl[start + 3], CANT_PARSE);
	}

	if (bl[start + 4].str != "in") {
		err_msg(bl[start + 4], CANT_PARSE);
	}

	if (bl[start + 5].type != NAME) {
		err_msg(bl[start + 5], CANT_PARSE);
	}

	if (bl[start + 6].str != "do") {
		err_msg(bl[start + 6], CANT_PARSE);
	}
}

void while_check(const Branch &token_list, int start, int end) {
	if (sz(start, end) < 4) {
		err_msg(token_list.branch_list[start], CANT_PARSE);
	}

	int then_pos = 0;
	for (int i = start + 1; i <= end; i++) {
		if (i == end) {
			err_msg(token_list.branch_list[start], CANT_PARSE);
		}

		const Branch &v = token_list.branch_list[i];
		if (v.str == "do") {
			then_pos = i;
			break;
		}
	}

	right_side_check(token_list, start + 1, then_pos - 1, false);
}

void typenew_member_left_side_check(const Branch &token_list,
int start, int end) {
	const std::vector<Branch> &bl = token_list.branch_list;

	if (bl[start].type != NAME) {
		err_msg(bl[start], CANT_PARSE);
	}

	if (bl[start + 1].str != ":") {
		err_msg(bl[start + 1], CANT_PARSE);
	}

	if (!is_var_type(bl[start + 2])) {
		err_msg(bl[start + 2], CANT_PARSE);
	}

	if (bl[start + 3].str != "=") {
		err_msg(bl[start + 3], CANT_PARSE);
	}
}

void lambda_check(const Branch &token_list, int start, int end) {
	const std::vector<Branch> &bl = token_list.branch_list;

	if (sz(start, end) < 6) {
		err_msg(bl[start], CANT_PARSE);
	}

	if (bl[start].str != "fn") {
		err_msg(bl[start], CANT_PARSE);
	}

	if (bl[start + 1].str != "(") {
		err_msg(bl[start + 1], CANT_PARSE);
	}

	int argv_start = start + 2;
	int argv_end = 0;
	for (int i = argv_start; i <= end; i++) {
		if (i == end) {
			err_msg(bl[argv_start], CANT_PARSE);
		}

		const Branch &v = bl[i];
		if (v.str == ")") {
			argv_end = i - 1;
			break;
		}
	}

	argument_list_check(token_list, argv_start, argv_end);

	if (bl[argv_end + 2].str != "->") {
		err_msg(bl[argv_end + 2], CANT_PARSE);
	}

	if (!is_return_type(bl[argv_end + 3])) {
		err_msg(bl[argv_end + 3], EXPECT_RETURN_TYPE);
	}

	if (bl[end].str != "end") {
		err_msg(bl[end], CANT_PARSE);
	}
}

void typenew_lambda_check(const Branch &token_list,int start,int end) {
	const std::vector<Branch> &bl = token_list.branch_list;

	if (sz(start, end) < 10) {
		err_msg(bl[start], CANT_PARSE);
	}

	typenew_member_left_side_check(token_list, start, end);
	
	lambda_check(token_list, start + 4, end);
}

void typenew_member_check(const Branch &token_list,int start,int end) {
	const std::vector<Branch> &bl = token_list.branch_list;

	if (sz(start, end) < 5) {
		err_msg(bl[start], CANT_PARSE);
	}

	typenew_member_left_side_check(token_list, start, end);
	right_side_check(token_list, start + 4, end, false);
}

void typenew_content_check(const Branch &token_list,int start,int end){
	int member_start = start;

	auto member_finished = [
		&token_list, &member_start
	](int member_end) -> void {
		if (token_list.branch_list[member_start + 2].str == "fn") {
			typenew_lambda_check(token_list, member_start, member_end);
		}
		else {
			typenew_member_check(token_list, member_start, member_end);
		}
	};

	for (int i = start; i <= end; i++) {
		const Branch &v = token_list.branch_list[i];
		if (v.str == ",") {
			member_finished(i - 1);
			member_start = i + 1;
		}
	}

	const Branch &last = token_list.branch_list[end];
	if (last.str != ",") {
		member_finished(end);
	}
}

void typenew_check(const Branch &token_list, int start, int end) {
	const std::vector<Branch> &bl = token_list.branch_list;

	if (sz(start, end) < 9) {
		err_msg(bl[start], CANT_PARSE);
	}
	
	if (bl[start + 1].type != NAME) {
		err_msg(bl[start + 1], CANT_PARSE);
	}

	int content_start = start + 3;
	int content_end = end - 1;
	if (bl[start + 2].str == "from") {
		content_start = start + 5;

		if (bl[start + 3].type != NAME) {
			err_msg(bl[start + 3], CANT_PARSE);
		}
	}

	if (bl[content_start - 1].str != "{") {
		err_msg(bl[content_start - 1], CANT_PARSE);
	}

	if (content_end < content_start) {
		err_msg(bl[content_start], CANT_HAVE_EMPTY_STRUCT);
	}
	
	typenew_content_check(token_list, content_start, content_end);
}

void return_check(const Branch &token_list, int start, int end) {
	if (start == end) {
		return;
	}

	std::cout << start << " " << end << std::endl;
	right_side_check(token_list, start + 1, end, false);
}

void lambda_new_check(const Branch &token_list, int start, int end) {
	const std::vector<Branch> &bl = token_list.branch_list;
	
	if (sz(start, end) < 11) {
		err_msg(bl[start], CANT_PARSE);
	}

	if (bl[start + 1].type != NAME) {
		err_msg(bl[start + 1], EXPECT_VAR_NAME);
	}

	if (bl[start + 2].str != ":") {
		err_msg(bl[start + 2], CANT_PARSE);
	}

	if (bl[start + 3].str != "fn") {
		err_msg(bl[start + 3], CANT_PARSE);
	}

	if (bl[start + 4].str != "=") {
		err_msg(bl[start + 4], CANT_PARSE);
	}

	int right_side_start = start + 5;

	lambda_check(token_list, right_side_start, end);
}

void lambda_assign_check(const Branch &token_list, int start, int end){
	const std::vector<Branch> &bl = token_list.branch_list;
	
	if (sz(start, end) < 8) {
		err_msg(bl[start], CANT_PARSE);
	}

	if (bl[start].type != NAME) {
		err_msg(bl[start], EXPECT_VAR_NAME);
	}

	if (bl[start + 1].str != "=") {
		err_msg(bl[start + 1], CANT_PARSE);
	}

	int right_side_start = start + 2;

	lambda_check(token_list, right_side_start, end);
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
		elseif_check(token_list, start, end);
	}
	else if (branch_type == FOR) {
		if (sz(start, end) < 8) {
			err_msg(token_list.branch_list[start], CANT_PARSE);
		}
		const Branch &plus2 = token_list.branch_list[start + 2];
		if (plus2.str == "=") {
			for_check(token_list, start, end);
		}
		else {
			foreach_check(token_list, start, end);
		}
	}
	else if (branch_type == WHILE) {
		while_check(token_list, start, end);
	}
	else if (branch_type == TYPE) {
		typenew_check(token_list, start, end);
	}
	else if (branch_type == RETURN) {
		return_check(token_list, start, end);
	}
	else if (branch_type == LAMBDA_NEW) {
		lambda_new_check(token_list, start, end);
	}
	else if (branch_type == LAMBDA_ASSIGN) {
		lambda_assign_check(token_list, start, end);
	}
}
