#include "group_by_operator.h"

#include "../types.h"
#include "shared.h"

namespace {
bool is_in_list(const std::vector<std::string> &op_str_list,
const std::string &str) {
	for (int i = 0; i < (int)op_str_list.size(); i++) {
		if (str == op_str_list[i]) {
			return true;
		}
	}
	return false;
}
}

void group_by_operator(Branch &bracket,
const std::vector<std::string> &op_str_list) {
	if (get_bracket_size(bracket) == 0) {
		return;
	}

	for (int i = 0; i < get_bracket_size(bracket); i++) {
		Branch &v = bracket.branch_list[i];
		if (is_bracket_type(v.type)) {
			group_by_operator(v, op_str_list);
		}
	}

	int i = 0;

	while (i < get_bracket_size(bracket)) {
		Branch v = bracket.branch_list[i];
		Branch nx;
		if (i + 1 < get_bracket_size(bracket)) {
			nx = bracket.branch_list[i + 1];
		}

		if (nx.type == OPERATOR && is_in_list(op_str_list, nx.str)) {
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
			i--;
		}
		
		i++;
	}
}
