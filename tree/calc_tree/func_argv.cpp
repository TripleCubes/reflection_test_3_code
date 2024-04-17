#include "func_argv.h"

#include "shared.h"
#include "../types.h"

namespace {
void argument_bracket_new(Branch &new_bracket,
const Branch &bracket, int start, int end) {
	Branch first = bracket.branch_list[start];

	new_bracket.type = BRACKET_ARGUMENT;
	new_bracket.line = first.line;
	new_bracket.column = first.column;

	for (int i = start; i <= end; i++) {
		Branch v = bracket.branch_list[i];
		new_bracket.branch_list.push_back(v);
	}
}
}

void func_argv(Branch &bracket) {
	for (int i = 0; i < get_bracket_size(bracket); i++) {
		Branch &v = bracket.branch_list[i];
		if (is_bracket_type(v.type)) {
			func_argv(v);
		}
	}

	int i = 0;
	int start = 0;

	auto argument_finished = [&i, &start, &bracket]() -> void {
		Branch argument;
		argument_bracket_new(argument, bracket, start, i);
		for (int j = start; j <= i + 1; j++) {
			branch_rm(bracket, start);
		}
		branch_add(bracket, start, argument);
		i = start;
		start++;
	};

	while (i < get_bracket_size(bracket)) {
		Branch nx;
		if (i + 1 < get_bracket_size(bracket)) {
			nx = bracket.branch_list[i + 1];
		}

		if (is_argv_bracket_type(bracket.type)
		&& nx.type == OPERATOR && nx.str == ",") {
			argument_finished();
		}

		i++;
	}

	if (is_argv_bracket_type(bracket.type)) {
		int bracket_sz = get_bracket_size(bracket);
		Branch argument;
		argument_bracket_new(argument, bracket, start, bracket_sz - 1);
		for (int j = start; j <= bracket_sz - 1; j++) {
			branch_rm(bracket, start);
		}
		branch_add(bracket, start, argument);
	}
}
