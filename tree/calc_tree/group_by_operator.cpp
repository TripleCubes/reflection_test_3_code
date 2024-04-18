#include "group_by_operator.h"

#include "../types.h"
#include "shared.h"
#include <iostream>

void group_by_operator(Branch &bracket, const std::string &op_str) {
	if (get_bracket_size(bracket) == 0) {
		return;
	}

	for (int i = 0; i < get_bracket_size(bracket); i++) {
		Branch &v = bracket.branch_list[i];
		if (is_bracket_type(v.type)) {
			group_by_operator(v, op_str);
		}
	}

	int i = 0;

	while (i < get_bracket_size(bracket)) {
		Branch v = bracket.branch_list[i];
		Branch nx;
		if (i + 1 < get_bracket_size(bracket)) {
			nx = bracket.branch_list[i + 1];
		}

		std::cout << to_str(v.type) << std::endl;
		if (nx.type == OPERATOR && nx.str == op_str) {
			Branch grouped;
			grouped.type = BRACKET_ROUND;
			grouped.line = v.line;
			grouped.column = v.column;
			
			for (int j = i; j < i + 3; j++) {
				Branch vv = bracket.branch_list[j];
				grouped.branch_list.push_back(vv);
			}

			for (int j = 0; j < 3; j++) {
				branch_rm(bracket, i);
			}

			branch_add(bracket, i, grouped);
		}
		
		i++;
	}
}
