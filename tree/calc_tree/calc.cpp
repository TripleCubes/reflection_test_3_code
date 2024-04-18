#include "calc.h"

#include "../types.h"
#include "shared.h"

void bracket_to_calc(Branch &bracket) {
	if (get_bracket_size(bracket) == 0) {
		return;
	}

	for (int i = 0; i < get_bracket_size(bracket); i++) {
		Branch &v = bracket.branch_list[i];
		if (is_bracket_type(v.type)) {
			bracket_to_calc(v);
		}
	}

	if (is_argv_bracket_type(bracket.type)) {
		return;
	}

	int i = 0;
	while (i < get_bracket_size(bracket)) {
		Branch v = bracket.branch_list[i];
		Branch nx;
		if (i + 1 < get_bracket_size(bracket)) {
			nx = bracket.branch_list[i + 1];
		}

		if (i == 0) {
			Branch calc;
			calc.type = CALC_START;
			calc.line = v.line;
			calc.column = v.column;
			calc.branch_list.push_back(v);

			branch_rm(bracket, i);
			branch_add(bracket, i, calc);
		}

		if (i != 0 && v.type == OPERATOR && nx.type != NONE
		&& nx.type != OPERATOR) {
			Branch calc;
			calc.type = CALC;
			calc.line = v.line;
			calc.column = v.column;
			calc.branch_list.push_back(v);
			calc.branch_list.push_back(nx);

			branch_rm(bracket, i);
			branch_rm(bracket, i);
			branch_add(bracket, i, calc);
		}

		i++;
	}
}
