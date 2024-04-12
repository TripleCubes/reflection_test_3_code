#include "reverse_operator.h"

#include "../types.h"
#include "shared.h"
#include <iostream>

void reverse_operator(Branch &bracket, const std::string &op,
BranchType reversed_type) {
	auto branch_rm = [&bracket](int i) -> void {
		bracket.branch_list.erase(
			bracket.branch_list.begin() + i
		);
	};

	auto branch_add = [&bracket](int i, Branch &branch) -> void {
		bracket.branch_list.insert(
			bracket.branch_list.begin() + i, branch
		);
	};

	int i = 0;
	while (i < get_bracket_size(bracket)) {
		Branch &v = bracket.branch_list[i];
		Branch nx;
		Branch prev;
		if (i + 1 < get_bracket_size(bracket)) {
			nx = bracket.branch_list[i + 1];
		}
		if (i - 1 >= 0) {
			prev = bracket.branch_list[i - 1];
		}

		if ((i == 0 || prev.type == OPERATOR)
		&& v.str == op && nx.type != NONE
		&& nx.type != OPERATOR) {
			Branch reversed;
			reversed.type = reversed_type;
			reversed.line = v.line;
			reversed.column = v.column;
			reversed.branch_list.push_back(nx);
			branch_add(i + 2, reversed);
			branch_rm(i + 1);
			branch_rm(i);
		}
		else if (is_bracket_type(v.type)) {
			reverse_operator(v, op, reversed_type);
		}

		i++;
	}
}
