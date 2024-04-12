#include "shared.h"

#include "../types.h"

int get_bracket_size(const Branch &bracket) {
	int sz = (int)bracket.branch_list.size();
	if (bracket.type == BRACKET_FUNCCALL) {
		return sz - 1;
	}
	return sz;
}

bool is_bracket_type(BranchType type) {
	if (type == BRACKET_ROUND
	|| type == BRACKET_SQUARE
	|| type == BRACKET_CURLY
	|| type == BRACKET_FUNCCALL) {
		return true;
	}
	return false;
}
