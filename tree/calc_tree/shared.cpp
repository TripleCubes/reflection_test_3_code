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

void branch_rm(Branch &bracket, int i) {
	bracket.branch_list.erase(
		bracket.branch_list.begin() + i
	);
}

void branch_add(Branch &bracket, int i, Branch &branch) {
	bracket.branch_list.insert(
		bracket.branch_list.begin() + i, branch
	);
}
