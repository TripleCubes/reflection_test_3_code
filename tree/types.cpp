#include "types.h"
#include <iostream>

namespace {
std::string spaces(int n) {
	std::string s;
	for (int i = 0; i < n; i++) {
		s += ' ';
	}
	return s;
}
}

std::string to_str(BranchType branch_type) {
	switch (branch_type) {
	case NONE: return "NONE"; break;

	case NUM: return "NUM"; break;
	case NAME: return "NAME"; break;
	case STR: return "STR"; break;
	case BRACKET: return "BRACKET"; break;
	case OPERATOR: return "OPERATOR"; break;
	case KEYWORD: return "KEYWORD"; break;

	default: return "UNHANDLED"; break;
	}

	return "UNHANDLED";
}

void print_branch(Branch branch, int indent) {
	std::string s = spaces(indent * 4);
	std::cout << s << to_str(branch.type) << std::endl;
	if (branch.str != "")
		std::cout << s << "    " << branch.str << std::endl;
	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		print_branch(branch.branch_list[i], indent + 1);
	}
}
