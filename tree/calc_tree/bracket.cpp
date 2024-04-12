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
	bool inside_bracket = false;
	int bracket_count = 0;
	int open_bracket_pos = 0;

	auto bracket_finished = [&open_bracket_pos,
	&inside_bracket, &result,
	&token_list](int close_bracket_pos) -> void {
		Branch bracket_start_token
		       = token_list.branch_list[open_bracket_pos + 1];
		
		Branch bracket;
		bracket.type = BRACKET_GROUP;
		bracket.line = bracket_start_token.line;
		bracket.column = bracket_start_token.column;
		to_bracket_tree(bracket, token_list,
			open_bracket_pos + 1, close_bracket_pos - 1);

		result.branch_list.push_back(bracket);

		inside_bracket = false;
	};

	for (int i = start_pos; i <= end_pos; i++) {
		Branch token = token_list.branch_list[i];
		
		if (!inside_bracket && is_open_bracket(token)) {
			inside_bracket = true;
			bracket_count = 0;
			open_bracket_pos = i;
		}

		if (inside_bracket) {
			if (is_matched_close_bracket(token, bracket_count)) {
				bracket_finished(i);
			}
		}
		else {
			result.branch_list.push_back(token);
		}
	}

	if (inside_bracket) {
		bracket_finished(end_pos);
	}
}
