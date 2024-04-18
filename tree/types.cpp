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

bool is_temp(BranchType type) {
	if (type == RIGHT_SIDE_TEMP
	|| type == ARGV_TEMP
	|| type == CONDITIONS_TEMP
	|| type == FOR_ITER_CONDITIONS_TEMP) {
		return true;
	}
	return false;
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

	case VARNEW: return "VARNEW"; break;
	case ASSIGN: return "ASSIGN"; break;
	case FUNCCALL: return "FUNCCALL"; break;
	case FUNCNEW: return "FUNCNEW"; break;
	case IF: return "IF"; break;
	case ELSEIF: return "ELSEIF"; break;
	case ELSE: return "ELSE"; break;
	case FOR: return "FOR"; break;
	case FOREACH: return "FOREACH"; break;
	case WHILE: return "WHILE"; break;
	case TYPE: return "TYPE"; break;

	case RIGHT_SIDE_TEMP: return "RIGHT_SIDE_TEMP"; break;
	case ARGV_TEMP: return "ARGV_TEMP"; break;
	case CONDITIONS_TEMP: return "CONDITIONS_TEMP"; break;
	case FOR_ITER_CONDITIONS_TEMP:
		return "FOR_ITER_CONDITIONS_TEMP";
		break;

//	case RIGHT_SIDE: return "RIGHT_SIDE"; break;
	case VAR_TYPE: return "VAR_TYPE"; break;
//	case ARGV: return "ARGV"; break;
	case FUNCNEW_ARGV: return "FUNCNEW_ARGV"; break;
	case RETURN_TYPES: return "RETURN_TYPES"; break;
	case CODE_BLOCK: return "CODE_BLOCK"; break;
//	case CONDITIONS: return "CONDITIONS"; break;
//	case FOR_ITER_CONDITIONS: return "FOR_ITER_CONDITIONS"; break;

	case BRACKET_ROUND: return "BRACKET_ROUND"; break;
	case BRACKET_SQUARE: return "BRACKET_SQUARE"; break;
	case BRACKET_CURLY: return "BRACKET_CURLY"; break;
	case BRACKET_FUNCCALL: return "BRACKET_FUNCCALL"; break;
	case BRACKET_ARGUMENT: return "BRACKET_ARGUMENT"; break;

	case REVERSE_SIGN: return "REVERSE_SIGN"; break;
	case REVERSE_COND: return "REVERSE_COND"; break;

	case CALC_START: return "CALC_START"; break;
	case CALC: return "CALC"; break;

	case ARRAY_INDEX: return "ARRAY_INDEX"; break;
	case TYPE_MEMBER: return "TYPE_MEMBER"; break;
	case TYPE_MEMBER_LIST: return "TYPE_MEMBER_LIST"; break;

	case RETURN: return "RETURN"; break;
	case BREAK: return "BREAK"; break;
	case CONTINUE: return "CONTINUE"; break;

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
		Branch sub_branch = branch.branch_list[i];
		
		if (is_temp(sub_branch.type)) { continue; }

		print_branch(sub_branch, indent + 1);
	}
}
