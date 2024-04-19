#include "tree_to_lua.h"

#include "../tree/types.h"
#include "../tree/calc_tree/shared.h"

namespace {
void code_block_to_str(std::string &result,
const Branch &code_block, int indent);

void str_indent(std::string &result, int indent) {
	for (int i = 0; i < indent * 4; i++) {
		result += ' ';
	}
}

void str_grouped_token(std::string &result, const Branch &token) {
	if ((int)token.branch_list.size() == 0) {
		result += token.str;
	} else {
		for (int i = 0; i < (int)token.branch_list.size(); i++) {
			Branch v = token.branch_list[i];
			result += v.str;
		}
	}
}

void str_bracket(std::string &result, const Branch &bracket) {
	for (int i = 0; i < get_bracket_size(bracket); i++) {
		const Branch &v = bracket.branch_list[i];

		if (v.type == BRACKET_ROUND) {
			if (i != 0) { result += " "; }
			result += "(";
			str_bracket(result, v);
			result += ")";
		}
		else if (v.type == BRACKET_SQUARE) {
			if (i != 0) { result += " "; }
			result += "[";
			str_bracket(result, v);
			result += "]";
		}
		else if (v.type == BRACKET_CURLY) {
			if (i != 0) { result += " "; }
			result += "{";
			str_bracket(result, v);
			result += "}";
		}
		else if (v.type == BRACKET_FUNCCALL) {
			int sz = (int)v.branch_list.size();
			Branch last = v.branch_list[sz - 1];
			
			str_grouped_token(result, last);
			result += "(";
			str_bracket(result, v);
			result += ")";
		}
		else if (v.type == BRACKET_ARGUMENT) {
			str_bracket(result, v);
			if (i != get_bracket_size(bracket) - 1) {
				result += ", ";
			}
		}

		else if (v.type == REVERSED) {
			int sz = (int)v.branch_list.size();
			Branch last = v.branch_list[sz - 1];
		
			if (i != 0) { result += " "; }
			result += last.str + " ";
			str_bracket(result, v);
		}

		else if (v.type == CALC_START) {
			const Branch &first = v.branch_list[0];
			if (!is_bracket_type(first.type)) {
				str_grouped_token(result, first);
			} else {
				str_bracket(result, v);
			}
		}
		else if (v.type == CALC) {
			const Branch &first = v.branch_list[0];
			const Branch &second = v.branch_list[1];
			result += " ";
			str_grouped_token(result, first);
			if (!is_bracket_type(second.type)) {
				result += " ";
				str_grouped_token(result, second);
			} else {
				str_bracket(result, v);
			}
		}
	}
}

void varnew_to_str(std::string &result, const Branch &branch) {
	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		Branch v = branch.branch_list[i];
		if (v.type == NAME) {
			str_grouped_token(result, v);
			result += " = ";
		}
		else if (v.type == BRACKET_ROUND) {
			str_bracket(result, v);
		}
	}
}

void assign_to_str(std::string &result, const Branch &branch) {
	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		Branch v = branch.branch_list[i];
		if (v.type == NAME) {
			str_grouped_token(result, v);
			result += " = ";
		}
		else if (v.type == BRACKET_ROUND) {
			str_bracket(result, v);
		}
	}
}

void funccall_to_str(std::string &result, const Branch &branch) {
	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		Branch v = branch.branch_list[i];
		if (v.type == NAME) {
			str_grouped_token(result, v);
			result += "(";
		}
		else if (v.type == BRACKET_FUNCCALL) {
			str_bracket(result, v);
			result += ")";
		}
	}
}

void str_funcnew_argv(std::string &result,
const Branch funcnew_argv) {
	for (int i = 0; i < (int)funcnew_argv.branch_list.size(); i++) {
		Branch v = funcnew_argv.branch_list[i];
		Branch name_token = v.branch_list[0];
		str_grouped_token(result, name_token);
		if (i != (int)funcnew_argv.branch_list.size() - 1) {
			result += ", ";
		}
	}
}

void funcnew_to_str(std::string &result, const Branch &branch,
int indent) {
	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		Branch v = branch.branch_list[i];
		if (v.type == NAME) {
			result += "function ";
			str_grouped_token(result, v);
			result += "(";
		}
		else if (v.type == FUNCNEW_ARGV) {
			str_funcnew_argv(result, v);
			result += ")";
		}
		else if (v.type == CODE_BLOCK) {
			result += '\n';
			code_block_to_str(result, v, indent + 1);
			str_indent(result, indent);
			result += "end";
		}
	}
}

void if_to_str(std::string &result, const Branch &branch,
int indent) {
	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		Branch v = branch.branch_list[i];
		if (v.type == BRACKET_ROUND) {
			result += "if ";
			str_bracket(result, v);
			result += " then";
		}
		else if (v.type == CODE_BLOCK) {
			result += '\n';
			code_block_to_str(result, v, indent + 1);
			str_indent(result, indent);
			result += "end";
		}
	}
}

void elseif_to_str(std::string &result, const Branch &branch) {
	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		Branch v = branch.branch_list[i];
		if (v.type == BRACKET_ROUND) {
			result += "elseif ";
			str_bracket(result, v);
			result += " then";
		}
	}
}

void for_iter_to_str(std::string &result, const Branch &branch,
int indent) {
	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		Branch v = branch.branch_list[i];
		if (v.type == NAME) {
			result += "for ";
			str_grouped_token(result, v);
			result += " = ";
		}
		else if (v.type == BRACKET_FUNCCALL) {
			str_bracket(result, v);
			result += " do";
		}
		else if (v.type == CODE_BLOCK) {
			result += '\n';
			code_block_to_str(result, v, indent + 1);
			str_indent(result, indent);
			result += "end";
		}
	}
}

void for_each_to_str(std::string &result, const Branch &branch,
int indent) {
	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		Branch v = branch.branch_list[i];
		if (i == 0) {
			result += "for ";
			str_grouped_token(result, v);
			result += ", ";
		}
		else if (i == 1) {
			str_grouped_token(result, v);
			result += " in ";
		}
		else if (i == 2) {
			result += "ipairs(";
			str_grouped_token(result, v);
			result += ") do";
		}
		else if (v.type == CODE_BLOCK) {
			result += '\n';
			code_block_to_str(result, v, indent + 1);
			str_indent(result, indent);
			result += "end";
		}
	}
}

void while_to_str(std::string &result, const Branch &branch,
int indent) {
	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		Branch v = branch.branch_list[i];
		if (v.type == BRACKET_ROUND) {
			result += "while ";
			str_bracket(result, v);
			result += " do";
		}
		else if (v.type == CODE_BLOCK) {
			result += '\n';
			code_block_to_str(result, v, indent + 1);
			str_indent(result, indent);
			result += "end";
		}
	}
}

void code_block_to_str(std::string &result,
const Branch &code_block, int indent) {
	for (int i = 0; i < (int)code_block.branch_list.size(); i++) {
		const Branch &v = code_block.branch_list[i];

		if (v.type == ELSEIF || v.type == ELSE) {
			str_indent(result, indent - 1);
		} else {
			str_indent(result, indent);
		}

		if (v.type == VARNEW) {
			varnew_to_str(result, v);
		}
		else if (v.type == ASSIGN) {
			assign_to_str(result, v);
		}
		else if (v.type == FUNCCALL) {
			funccall_to_str(result, v);
		}
		else if (v.type == FUNCNEW) {
			funcnew_to_str(result, v, indent);
		}
		else if (v.type == IF) {
			if_to_str(result, v, indent);
		}
		else if (v.type == ELSEIF) {
			elseif_to_str(result, v);
		}
		else if (v.type == ELSE) {
			result += "else";
		}
		else if (v.type == FOR) {
			for_iter_to_str(result, v, indent);
		}
		else if (v.type == FOREACH) {
			for_each_to_str(result, v, indent);
		}
		else if (v.type == WHILE) {
			while_to_str(result, v, indent);
		}

		result += '\n';
	}
}
}

void tree_to_lua(std::string &result, const Branch &tree) {
	code_block_to_str(result, tree, 0);
}
