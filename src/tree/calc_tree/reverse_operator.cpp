#include "reverse_operator.h"

#include "../types.h"
#include "shared.h"
#include <iostream>

void reverse_operator(Branch &bracket, const std::string &op) {
	if (get_bracket_size(bracket) == 0) {
		return;
	}

	for (int i = 0; i < get_bracket_size(bracket); i++) {
		Branch &v = bracket.branch_list[i];
		if (is_bracket_type(v.type)) {
			reverse_operator(v, op);
		}
	}

	int i = 0;
	while (i < get_bracket_size(bracket)) {
		Branch v = bracket.branch_list[i];
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
			reversed.type = REVERSED;
			reversed.line = v.line;
			reversed.column = v.column;
			reversed.branch_list.push_back(nx);
			reversed.branch_list.push_back(v);
			branch_add(bracket, i + 2, reversed);
			branch_rm(bracket, i + 1);
			branch_rm(bracket, i);
		}

		i++;
	}
}
