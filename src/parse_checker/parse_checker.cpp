#include "parse_checker.h"

#include "../tree/types.h"
#include "../err_msg/err_msg.h"
#include "../tree/var_types.h"
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
	if (is_value(nx) || is_reverse_op(nx) || nx.str == ")") {
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

bool right_side_check(const Branch &token_list, int start, int end,
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
			return false;
		}

		if (!in_funccall) {
			if (is_value(v) && !is_next_to_value(nx)) {
				err_msg(v, EXPECT_OP);
				return false;
			}

			if (is_op(v) && !is_next_to_op(nx, nx_nx)) {
				err_msg(v, EXPECT_VALUE);
				return false;
			}

			if (v.str == "(" && !is_next_to_open_bracket(nx)) {
				err_msg(v, EXPECT_VALUE);
				return false;
			}

			if (v.str == ")" && !is_next_to_close_bracket(nx)) {
				err_msg(v, EXPECT_OP);
				return false;
			}
		}

		if (in_funccall) {
			if (v.str == "(" && !is_next_to_open_bracket(nx)) {
				err_msg(v, EXPECT_VALUE);
				return false;
			}
			
			if (is_value(v) && !f_is_next_to_value(nx)) {
				err_msg(v, EXPECT_OP);
				return false;
			}

			if (is_op(v) && !is_next_to_op(nx, nx_nx)) {
				err_msg(v, EXPECT_VALUE);
				return false;
			}

			if (v.str == ")"
			&& !f_is_next_to_close_bracket(nx, f_bracket_count)) {
				err_msg(v, EXPECT_OP);
				return false;
			}

			if (v.str == "," && !f_is_next_to_comma(nx)) {
				err_msg(v, EXPECT_VALUE);
				return false;
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
				return false;
			}
		}
	}
	
	const Branch &end_token = token_list.branch_list[end];
	if (bracket_count > 0) {
		err_msg(end_token, EXPECT_CLOSE_ROUND_BRACKET);
		return false;
	}

	return true;
}

bool varnew_check(const Branch &token_list, int start, int end) {
	int equal_sign_pos = start + 4;
	if (sz(start, end) < 6) {
		err_msg(token_list.branch_list[start], CANT_PARSE);
		return false;
	}
	if (token_list.branch_list[start + 1].type != NAME) {
		err_msg(token_list.branch_list[start + 1], EXPECT_VAR_NAME);
		return false;
	}
	if (token_list.branch_list[start + 2].str != ":") {
		err_msg(token_list.branch_list[start + 2], EXPECT_COLON);
		return false;
	}
	if (!is_var_type(token_list.branch_list[start + 3])) {
		err_msg(token_list.branch_list[start + 3], EXPECT_VAR_TYPE);
		return false;
	}
	if (token_list.branch_list[start + 4].str == "[") {
		equal_sign_pos = start + 6;
		if (sz(start, end) < 8) {
			err_msg(token_list.branch_list[start], CANT_PARSE);
			return false;
		}
		if (token_list.branch_list[start + 5].str != "]") {
			err_msg(token_list.branch_list[start],
			        EXPECT_CLOSE_SQUARE_BRACKET);
			return false;
		}
	}
	if (token_list.branch_list[equal_sign_pos].str != "=") {
		err_msg(token_list.branch_list[start], EXPECT_EQUAL_SIGN);
		return false;
	}
	
	if (token_list.branch_list[equal_sign_pos + 1].str == "{") {
		if (token_list.branch_list[end].str != "}"
		|| end != equal_sign_pos + 2) {
			err_msg(token_list.branch_list[equal_sign_pos + 1],
					EXPECT_CLOSE_CURLY_BRACKET);
			return false;
		}
		else {
			return true;
		}
	}

	if (token_list.branch_list[equal_sign_pos + 1].str == "[") {
		if (token_list.branch_list[end].str != "]") {
			err_msg(token_list.branch_list[end],
			        EXPECT_CLOSE_SQUARE_BRACKET);
			return false;
		}
		else {
			bool check_result
			= right_side_check(token_list,equal_sign_pos+2,end-1,true);
			if (check_result == false) {
				return false;
			}
			return true;
		}
	}

	bool check_result
		= right_side_check(token_list, equal_sign_pos + 1, end, false);
	if (check_result == false) {
		return false;
	}

	return true;
}

bool assign_check(const Branch &token_list, int start, int end) {
	if (sz(start, end) < 3) {
		err_msg(token_list.branch_list[start], CANT_PARSE);
		return false;
	}

	if (token_list.branch_list[start].type != NAME) {
		err_msg(token_list.branch_list[start], EXPECT_VAR_NAME);
		return false;
	}

	if (token_list.branch_list[start + 1].str != "=") {
		err_msg(token_list.branch_list[start + 1], EXPECT_EQUAL_SIGN);
		return false;
	}

	int equal_sign_pos = start + 1;
	
	if (token_list.branch_list[equal_sign_pos + 1].str == "{") {
		err_msg(token_list.branch_list[equal_sign_pos + 1],
		        OPEN_CURLY_IN_ASSIGN);
		return false;
	}

	if (token_list.branch_list[equal_sign_pos + 1].str == "[") {
		if (token_list.branch_list[end].str != "]") {
			err_msg(token_list.branch_list[end],
			        EXPECT_CLOSE_SQUARE_BRACKET);
			return false;
		}
		else {
			bool check_result
			= right_side_check(token_list,equal_sign_pos+2,end-1,true);
			if (check_result == false) {
				return false;
			}
			return true;
		}
	}

	bool check_result
		= right_side_check(token_list, equal_sign_pos + 1, end, false);
	if (check_result == false) {
		return false;
	}

	return true;
}

bool funccall_check(const Branch &token_list, int start, int end) {
	if (sz(start, end) < 3) {
		err_msg(token_list.branch_list[start], CANT_PARSE);
		return false;
	}

	if (token_list.branch_list[start].type != NAME) {
		err_msg(token_list.branch_list[start], EXPECT_FUNC_NAME);
		return false;
	}

	if (token_list.branch_list[start + 1].str != "(") {
		err_msg(token_list.branch_list[start + 1],
		        EXPECT_OPEN_ROUND_BRACKET);
		return false;
	}

	if (token_list.branch_list[end].str != ")") {
		err_msg(token_list.branch_list[end],
		        EXPECT_CLOSE_ROUND_BRACKET);
		return false;
	}

	bool check_result
		= right_side_check(token_list, start + 2, end - 1, true);
	if (check_result == false) {
		return false;
	}
	return true;
}

bool argument_list_check(const Branch &token_list, int start, int end){
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
			return false;
		}

		if (v.str == ":" && !is_var_type(nx)) {
			err_msg(v, EXPECT_VAR_TYPE);
			return false;
		}

		if (is_var_type(v) && prev.str == ":" && !(nx.str == ","
		|| nx.str == "[" || nx.type == NONE)) {
			err_msg(v, EXPECT_COMMA);
			return false;
		}

		if (v.str == "[" && nx.str != "]") {
			err_msg(v, EXPECT_CLOSE_SQUARE_BRACKET);
			return false;
		}

		if (v.str == "]" && !(nx.str == ","
		|| nx.type == NONE)) {
			err_msg(v, EXPECT_COMMA);
			return false;
		}

		if (v.str == "," && nx.type != NAME) {
			err_msg(v, EXPECT_VAR_NAME);
			return false;
		}
	}
	return true;
}

bool funcnew_check(const Branch &token_list, int start, int end) {
	if (sz(start, end) < 7) {
		err_msg(token_list.branch_list[start], CANT_PARSE);
		return false;
	}

	if (token_list.branch_list[start + 1].type != NAME) {
		err_msg(token_list.branch_list[start + 1], EXPECT_FUNC_NAME);
		return false;
	}

	if (token_list.branch_list[start + 2].str != "(") {
		err_msg(token_list.branch_list[start + 2],
		        EXPECT_OPEN_ROUND_BRACKET);
		return false;
	}

	int bracket_close = 0;
	for (int i = start + 3; i <= end; i++) {
		if (i == end) {
			err_msg(token_list.branch_list[start], CANT_PARSE);
			return false;
		}

		const Branch &v = token_list.branch_list[i];
		if (v.str == ")") {
			bracket_close = i;
			break;
		}
	}

	bool check_result
	= argument_list_check(token_list, start + 3, bracket_close - 1);
	if (check_result == false) {
		return false;
	}

	if (end - start < bracket_close - start + 3) {
		err_msg(token_list.branch_list[start], CANT_PARSE);
		return false;
	}

	if (token_list.branch_list[bracket_close + 1].str != "->") {
		err_msg(token_list.branch_list[bracket_close+1], EXPECT_ARROW);
		return false;
	}

	if (!is_return_type(token_list.branch_list[bracket_close + 2])) {
		err_msg(token_list.branch_list[bracket_close+1],
		        EXPECT_RETURN_TYPE);
		return false;
	}

	if (token_list.branch_list[bracket_close + 3].str == ",") {
		err_msg(token_list.branch_list[bracket_close + 3],
		        MULTI_RETURN_UNSUPPORTED);
		return false;
	}

	return true;
}

bool if_check(const Branch &token_list, int start, int end) {
	if (sz(start, end) < 4) {
		err_msg(token_list.branch_list[start], CANT_PARSE);
		return false;
	}

	int then_pos = 0;
	for (int i = start + 1; i <= end; i++) {
		if (i == end) {
			err_msg(token_list.branch_list[start], EXPECT_THEN);
			return false;
		}

		const Branch &v = token_list.branch_list[i];
		if (v.str == "then") {
			then_pos = i;
			break;
		}
	}

	bool check_result
		= right_side_check(token_list, start + 1, then_pos - 1, false);
	if (check_result == false) {
		return false;
	}
	
	return true;
}

bool elseif_check(const Branch &token_list, int start, int end) {
	if (sz(start, end) < 3) {
		err_msg(token_list.branch_list[start], CANT_PARSE);
		return false;
	}

	for (int i = start + 1; i <= end; i++) {
		const Branch &v = token_list.branch_list[i];
		if (v.str == "then") {
			break;
		}

		if (i == end) {
			err_msg(token_list.branch_list[start], EXPECT_THEN);
			return false;
		}
	}

	return true;
}

bool for_check(const Branch &token_list, int start, int end) {
	const std::vector<Branch> &bl = token_list.branch_list;

	if (bl[start + 1].type != NAME) {
		err_msg(bl[start + 1], EXPECT_VAR_NAME);
		return false;
	}

	if (bl[start + 2].str != "=") {
		err_msg(bl[start + 2], EXPECT_EQUAL_SIGN);
		return false;
	}

	int right_side_start = start + 3;
	int right_side_end = 0;
	for (int i = right_side_start; i <= end; i++) {
		if (i == end) {
			err_msg(bl[right_side_start], EXPECT_DO);
			return false;
		}

		if (bl[i].str == "do") {
			right_side_end = i - 1;
			break;
		}
	}

	bool check_result
	=right_side_check(token_list,right_side_start,right_side_end,true);

	if (check_result == false) {
		return false;
	}

	return true;
}

bool foreach_check(const Branch &token_list, int start, int end) {
	const std::vector<Branch> &bl = token_list.branch_list;

	if (bl[start + 1].type != NAME) {
		err_msg(bl[start + 1], EXPECT_VAR_NAME);
		return false;
	}

	if (bl[start + 2].str != ",") {
		err_msg(bl[start + 2], EXPECT_COMMA);
		return false;
	}

	if (bl[start + 3].type != NAME) {
		err_msg(bl[start + 3], EXPECT_VAR_NAME);
		return false;
	}

	if (bl[start + 4].str != "in") {
		err_msg(bl[start + 4], EXPECT_IN_KEYWORD);
		return false;
	}

	if (bl[start + 5].type != NAME) {
		err_msg(bl[start + 5], EXPECT_VAR_NAME);
		return false;
	}

	if (bl[start + 6].str != "do") {
		err_msg(bl[start + 6], EXPECT_DO);
		return false;
	}

	return true;
}

bool while_check(const Branch &token_list, int start, int end) {
	if (sz(start, end) < 4) {
		err_msg(token_list.branch_list[start], CANT_PARSE);
		return false;
	}

	int then_pos = 0;
	for (int i = start + 1; i <= end; i++) {
		if (i == end) {
			err_msg(token_list.branch_list[start], EXPECT_DO);
			return false;
		}

		const Branch &v = token_list.branch_list[i];
		if (v.str == "do") {
			then_pos = i;
			break;
		}
	}

	bool check_result
		= right_side_check(token_list, start + 1, then_pos - 1, false);
	if (check_result == false) {
		return false;
	}

	return true;
}

bool typenew_member_left_side_check(const Branch &token_list,
int start, int end) {
	const std::vector<Branch> &bl = token_list.branch_list;

	if (bl[start].type != NAME) {
		err_msg(bl[start], EXPECT_VAR_NAME);
		return false;
	}

	if (bl[start + 1].str != ":") {
		err_msg(bl[start + 1], EXPECT_COLON);
		return false;
	}

	if (!is_var_type(bl[start + 2])) {
		err_msg(bl[start + 2], EXPECT_VAR_TYPE);
		return false;
	}

	if (bl[start + 3].str != "=") {
		err_msg(bl[start + 3], EXPECT_EQUAL_SIGN);
		return false;
	}

	return true;
}

bool lambda_check(const Branch &token_list, int start, int end) {
	const std::vector<Branch> &bl = token_list.branch_list;

	if (sz(start, end) < 6) {
		err_msg(bl[start], CANT_PARSE);
		return false;
	}

	if (bl[start].str != "fn") {
		err_msg(bl[start], EXPECT_FN_KEYWORD);
		return false;
	}

	if (bl[start + 1].str != "(") {
		err_msg(bl[start + 1], EXPECT_OPEN_ROUND_BRACKET);
		return false;
	}

	int argv_start = start + 2;
	int argv_end = 0;
	for (int i = argv_start; i <= end; i++) {
		if (i == end) {
			err_msg(bl[argv_start], EXPECT_CLOSE_ROUND_BRACKET);
			return false;
		}

		const Branch &v = bl[i];
		if (v.str == ")") {
			argv_end = i - 1;
			break;
		}
	}

	bool check_result
		= argument_list_check(token_list, argv_start, argv_end);
	if (check_result == false) {
		return false;
	}

	if (bl[argv_end + 2].str != "->") {
		err_msg(bl[argv_end + 2], EXPECT_ARROW);
		return false;
	}

	if (!is_return_type(bl[argv_end + 3])) {
		err_msg(bl[argv_end + 3], EXPECT_RETURN_TYPE);
		return false;
	}

	if (bl[argv_end + 4].str == ",") {
		err_msg(bl[argv_end + 4], MULTI_RETURN_UNSUPPORTED);
		return false;
	}

	if (bl[end].str != "end") {
		err_msg(bl[end], EXPECT_END);
		return false;
	}

	return true;
}

bool typenew_lambda_check(const Branch &token_list,int start,int end) {
	const std::vector<Branch> &bl = token_list.branch_list;

	if (sz(start, end) < 10) {
		err_msg(bl[start], CANT_PARSE);
		return false;
	}

	bool member_check_result
		= typenew_member_left_side_check(token_list, start, end);
	if (member_check_result == false) {
		return false;
	}

	bool lambda_check_result
		= lambda_check(token_list, start + 4, end);
	if (lambda_check_result == false) {
		return false;
	}

	return true;
}

bool typenew_member_check_primitive(const Branch &token_list,
int start,int end) {
	const std::vector<Branch> &bl = token_list.branch_list;

	if (sz(start, end) < 5) {
		err_msg(bl[start], CANT_PARSE);
		return false;
	}

	bool member_check_result
		= typenew_member_left_side_check(token_list, start, end);
	if (member_check_result == false) {
		return false;
	}

	bool right_side_check_result
		= right_side_check(token_list, start + 4, end, false);
	if (right_side_check_result == false) {
		return false;
	}

	return true;
}

bool typenew_member_check_struct(const Branch &token_list,
int start,int end) {
	const std::vector<Branch> &bl = token_list.branch_list;

	if (sz(start, end) < 6) {
		err_msg(bl[start], CANT_PARSE);
		return false;
	}

	bool check_result
		= typenew_member_left_side_check(token_list, start, end);
	if (check_result == false) {
		return false;
	}
	
	if (bl[start + 4].str != "{") {
		err_msg(bl[start + 4], EXPECT_OPEN_CURLY_BRACKET);
		return false;
	}
	
	if (bl[start + 5].str != "}") {
		err_msg(bl[start + 5], EXPECT_CLOSE_CURLY_BRACKET);
		return false;
	}

	return true;
}

bool is_inc_code_block_count(const Branch &token,const Branch &minus2){
	if (token.str == "if" || token.str == "for"
	|| token.str == "while"
	|| (token.str == "fn" && minus2.str != "fn")) {
		return true;
	}
	return false;
}

bool typenew_content_check(const Branch &token_list,int start,int end){
	int code_block_count = 0;

	int member_start = start;

	auto member_finished = [
		&token_list, &member_start
	](int member_end) -> bool {
		const std::string &type
			= token_list.branch_list[member_start + 2].str;

		if (type == "fn") {
			bool check_result
			=typenew_lambda_check(token_list,member_start,member_end);
			if (check_result == false) {
				return false;
			}
		}
		else if (is_primitive(type)) {
			bool check_result
			= typenew_member_check_primitive(token_list,
				member_start, member_end);
			if (check_result == false) {
				return false;
			}
		}
		else {
			bool check_result
			= typenew_member_check_struct(token_list,
				member_start, member_end);
			if (check_result == false) {
				return false;
			}
		}

		return true;
	};

	for (int i = start; i <= end; i++) {
		const Branch &v = token_list.branch_list[i];
		Branch nx_nx;
		if (i + 2 <= end) {
			nx_nx = token_list.branch_list[i + 2];
		}
		Branch prev_2;
		if (i - 2 >= start) {
			prev_2 = token_list.branch_list[i - 2];
		}

		if (is_inc_code_block_count(v, prev_2)) {
			code_block_count++;
		}
		if (v.str == "end") {
			code_block_count--;
		}

		if (code_block_count < 0) {
			err_msg(v, CANT_PARSE);
			return false;
		}

		if (code_block_count != 0) {
			continue;
		}

		if (v.str == "," && nx_nx.str == ":") {
			bool finished_result = member_finished(i - 1);
			if (finished_result == false) {
				return false;
			}
			member_start = i + 1;
		}
	}

	const Branch &last = token_list.branch_list[end];
	if (last.str != ",") {
		bool finished_result = member_finished(end);
		if (finished_result == false) {
			return false;
		}
	}

	return true;
}

bool typenew_check(const Branch &token_list, int start, int end) {
	const std::vector<Branch> &bl = token_list.branch_list;

	if (sz(start, end) < 9) {
		err_msg(bl[start], CANT_PARSE);
		return false;
	}
	
	if (bl[start + 1].type != NAME) {
		err_msg(bl[start + 1], EXPECT_TYPE_NAME);
		return false;
	}

	int content_start = start + 3;
	int content_end = end - 1;
	if (bl[start + 2].str == "from") {
		content_start = start + 5;

		if (bl[start + 3].type != NAME) {
			err_msg(bl[start + 3], EXPECT_TYPE_NAME);
			return false;
		}
	}

	if (bl[content_start - 1].str != "{") {
		err_msg(bl[content_start - 1], EXPECT_OPEN_CURLY_BRACKET);
		return false;
	}

	if (content_end < content_start) {
		err_msg(bl[content_start], CANT_HAVE_EMPTY_STRUCT);
		return false;
	}
	
	bool check_result
	= typenew_content_check(token_list, content_start, content_end);
	if (check_result == false) {
		return false;
	}

	return true;
}

bool return_check(const Branch &token_list, int start, int end) {
	if (start == end) {
		return true;
	}

	bool check_result
		= right_side_check(token_list, start + 1, end, false);
	if (check_result == false) {
		return false;
	}

	return true;
}

bool lambda_new_check(const Branch &token_list, int start, int end) {
	const std::vector<Branch> &bl = token_list.branch_list;
	
	if (sz(start, end) < 11) {
		err_msg(bl[start], CANT_PARSE);
		return false;
	}

	if (bl[start + 1].type != NAME) {
		err_msg(bl[start + 1], EXPECT_VAR_NAME);
		return false;
	}

	if (bl[start + 2].str != ":") {
		err_msg(bl[start + 2], EXPECT_COLON);
		return false;
	}

	if (bl[start + 3].str != "fn") {
		err_msg(bl[start + 3], EXPECT_FN_KEYWORD);
		return false;
	}

	if (bl[start + 4].str != "=") {
		err_msg(bl[start + 4], EXPECT_EQUAL_SIGN);
		return false;
	}

	int right_side_start = start + 5;

	bool check_result
		= lambda_check(token_list, right_side_start, end);
	if (check_result == false) {
		return false;
	}

	return true;
}

bool lambda_assign_check(const Branch &token_list, int start, int end){
	const std::vector<Branch> &bl = token_list.branch_list;
	
	if (sz(start, end) < 8) {
		err_msg(bl[start], CANT_PARSE);
		return false;
	}

	if (bl[start].type != NAME) {
		err_msg(bl[start], EXPECT_VAR_NAME);
		return false;
	}

	if (bl[start + 1].str != "=") {
		err_msg(bl[start + 1], EXPECT_EQUAL_SIGN);
		return false;
	}

	int right_side_start = start + 2;

	bool check_result
		= lambda_check(token_list, right_side_start, end);
	if (check_result == false) {
		return false;
	}

	return true;
}
}

bool parse_check(const Branch &token_list, BranchType branch_type,
int start, int end) {
	if (branch_type == VARNEW) {
		return varnew_check(token_list, start, end);
	}
	else if (branch_type == ASSIGN) {
		return assign_check(token_list, start, end);
	}
	else if (branch_type == FUNCCALL) {
		return funccall_check(token_list, start, end);
	}
	else if (branch_type == FUNCNEW) {
		return funcnew_check(token_list, start, end);
	}
	else if (branch_type == IF) {
		return if_check(token_list, start, end);
	}
	else if (branch_type == ELSEIF) {
		return elseif_check(token_list, start, end);
	}
	else if (branch_type == FOR) {
		if (sz(start, end) < 8) {
			err_msg(token_list.branch_list[start], CANT_PARSE);
			return false;
		}
		const Branch &plus2 = token_list.branch_list[start + 2];
		if (plus2.str == "=") {
			return for_check(token_list, start, end);
		}
		else {
			return foreach_check(token_list, start, end);
		}
	}
	else if (branch_type == WHILE) {
		return while_check(token_list, start, end);
	}
	else if (branch_type == TYPE) {
		return typenew_check(token_list, start, end);
	}
	else if (branch_type == RETURN) {
		return return_check(token_list, start, end);
	}
	else if (branch_type == LAMBDA_NEW) {
		return lambda_new_check(token_list, start, end);
	}
	else if (branch_type == LAMBDA_ASSIGN) {
		return lambda_assign_check(token_list, start, end);
	}

	return true;
}
