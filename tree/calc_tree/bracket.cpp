#include "bracket.h"

#include "../types.h"

namespace {
bool is_open_bracket(const Branch &token) {
	if (token.type != BRACKET) {
		return false;
	}

	if (token.str != "(" && token.str != "[" && token.str != "{") {
		return false;
	}

	return true;
}

bool is_close_bracket(const Branch &token) {
	if (token.type != BRACKET) {
		return false;
	}

	if (token.str != ")" && token.str != "]" && token.str != "}") {
		return false;
	}

	return true;
}

int is_matched_close_bracket(const Branch &token, int &bracket_count) {
	if (is_open_bracket(token)) { bracket_count++; }
	else if (is_close_bracket(token)) { bracket_count--; }

	if (bracket_count == 0) {
		return true;
	}
	return false;
}
}

void to_bracket_tree(Branch &result, const Branch &token_list,
int start_pos, int end_pos) {
	int bracket_count = 0;
	int open_bracket_pos = 0;
	BranchType bracket_type = NONE;
	Branch func_name;

	auto bracket_finished = [&open_bracket_pos, &bracket_type,
	&func_name,
	&result, &token_list](int close_bracket_pos) -> void {
		Branch bracket_start_token
		       = token_list.branch_list[open_bracket_pos + 1];
		
		Branch bracket;
		bracket.type = bracket_type;
		bracket.line = bracket_start_token.line;
		bracket.column = bracket_start_token.column;
		to_bracket_tree(bracket, token_list,
			open_bracket_pos + 1, close_bracket_pos - 1);
		if (bracket_type == BRACKET_FUNCCALL) {
			bracket.branch_list.push_back(func_name);
		}

		result.branch_list.push_back(bracket);

		bracket_type = NONE;
	};

	for (int i = start_pos; i <= end_pos; i++) {
		Branch token = token_list.branch_list[i];
		Branch prev_token;
		if (i - 1 >= start_pos) {
			prev_token = token_list.branch_list[i - 1];
		}
		Branch nx_token;
		if (i + 1 <= end_pos) {
			nx_token = token_list.branch_list[i + 1];
		}
		
		if (bracket_type == NONE && is_open_bracket(token)) {
			if (prev_token.type == NAME && token.str == "(") {
				bracket_type = BRACKET_FUNCCALL;
				func_name = prev_token;
			} else if (token.str == "(") {
				bracket_type = BRACKET_ROUND;
			} else if (token.str == "[") {
				bracket_type = BRACKET_SQUARE;
			} else if (token.str == "{") {
				bracket_type = BRACKET_CURLY;
			}
			bracket_count = 0;
			open_bracket_pos = i;
		}

		if (bracket_type != NONE) {
			if (is_matched_close_bracket(token, bracket_count)) {
				bracket_finished(i);
			}
		}
		else {
			bool cond = true;
			if (token.type == NAME && nx_token.str == "(") {
				cond = false;
			}
			if (cond) {
				result.branch_list.push_back(token);
			}
		}
	}

	if (bracket_type != NONE) {
		bracket_finished(end_pos);
	}
}
