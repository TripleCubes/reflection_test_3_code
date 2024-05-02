#include "tree_to_lua.h"

#include "../tree/types.h"
#include "../tree/calc_tree/shared.h"
#include "../lib_func/lib_func.h"
#include <vector>
#include <iostream>

namespace {
const std::string TYPEDEFL_BEGIN = "__default_";

void code_block_to_str(std::string &result,
const Branch &code_block, int indent);

void str_indent(std::string &result, int indent) {
	for (int i = 0; i < indent * 4; i++) {
		result += ' ';
	}
}

void str_grouped_token(std::string &result, const Branch &token) {
	if (token.str != "") {
		result += token.str;
		if ((int)token.branch_list.size() != 0) {
			if (token.branch_list[0].type == ARRAY_INDEX) {
				result += "[]";
			}
		}
	} else {
		for (int i = 0; i < (int)token.branch_list.size(); i++) {
			Branch v = token.branch_list[i];
			result += v.str;
		}
	}
}

void last_dot_to_colon(Branch &token) {
	std::string name;
	str_grouped_token(name, token);
	std::string lib_func_return_type = get_lib_func_return_type(name);
	if (lib_func_return_type != "") {
		return;
	}

	int dot_pos = 0;

	for (int i = 0; i < (int)token.branch_list.size(); i++) {
		const Branch &v = token.branch_list[i];
		if (v.str == ".") {
			dot_pos = i;
		}
	}

	if (dot_pos == 0) {
		return;
	}

	token.branch_list[dot_pos].str = ":";
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
			result += "{";
			str_bracket(result, v);
			result += "}";
		}
		else if (v.type == BRACKET_CURLY) {
			if (i != 0) { result += " "; }
			result += "{";
			str_bracket(result, v);
			result += "}";
		}
		else if (v.type == BRACKET_FUNCCALL) {
			if (i != 0) { result += " "; }
			int sz = (int)v.branch_list.size();
			Branch last = v.branch_list[sz - 1];
			last_dot_to_colon(last);
			
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
				//result += " ";
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
			Branch var_type = branch.branch_list[1];
			Branch calc_start_branch = v.branch_list[0].branch_list[0];
			if (calc_start_branch.type == BRACKET_CURLY) {
				result += TYPEDEFL_BEGIN + var_type.str;
				result += "()";
			}
			else {
				str_bracket(result, v);
			}
		}
	}
}

void assign_to_str(std::string &result, const Branch &branch) {
	std::string var_name;
	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		Branch v = branch.branch_list[i];
		if (v.type == NAME) {
			str_grouped_token(var_name, v);
			result += var_name;
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
			last_dot_to_colon(v);
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
			result += "end\n";
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

void lambda_to_str(std::string &result, const Branch &branch,
int indent);
void lambda_to_str_typenew(std::string &result, const Branch &branch,
int indent);

void type_to_str(std::string &result, const Branch &branch,
int indent) {
	Branch name;
	Branch inherit;
	Branch type_member_list;
	std::vector<std::string> param_list;
	std::vector<std::string> type_list;
	std::vector<Branch> defl_list;

	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		Branch v = branch.branch_list[i];
		if (i == 0) {
			name = v;
		}
		else if (i == 1) {
			inherit = v;
		}
		else if (v.type == TYPE_MEMBER_LIST) {
			type_member_list = v;
		}
	}

	for (int i = 0; i < (int)type_member_list.branch_list.size(); i++){
		Branch v = type_member_list.branch_list[i];
		Branch name = v.branch_list[0];
		Branch type = v.branch_list[1];
		Branch defl = v.branch_list[3];
		std::string param;
		str_grouped_token(param, name);
		param_list.push_back(param);
		
		std::string type_str;
		str_grouped_token(type_str, type);
		type_list.push_back(type_str);

		defl_list.push_back(defl);
	}

	result += "function " + TYPEDEFL_BEGIN;
	str_grouped_token(result, name);
	result += "()\n";
	str_indent(result, indent + 1);
	result += "local a = ";

	if (inherit.type == NONE) {
		result += "{}\n";
	} else {
		result += TYPEDEFL_BEGIN;
		str_grouped_token(result, inherit);
		result += "()\n";
	}

	for (int i = 0; i < (int)param_list.size(); i++) {
		const std::string &param = param_list[i];
		const std::string &type = type_list[i];
		const Branch &defl = defl_list[i];
		const Branch &member = type_member_list.branch_list[i];

		str_indent(result, indent + 1);

		if (type == "fn") {
			lambda_to_str_typenew(result, member, indent + 1);
			result += "\n";
			continue;
		}

		Branch calc_start_branch = defl.branch_list[0].branch_list[0];
		if (calc_start_branch.type == BRACKET_CURLY) {
			result += "a." + param + " = ";
			result += TYPEDEFL_BEGIN + type + "()";
		}
		else {
			result += "a." + param + " = ";
			str_bracket(result, defl);
		}
		result += "\n";
	}
	str_indent(result, indent + 1);
	result += "return a\n";
	str_indent(result, indent);
	result += "end\n";
}

void return_to_str(std::string &result, const Branch &branch) {
	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		Branch v = branch.branch_list[i];
		if (v.type == BRACKET_ROUND) {
			result += "return ";
			str_bracket(result, v);
		}
	}
}

void lambda_to_str_typenew(std::string &result, const Branch &branch,
int indent) {
	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		Branch v = branch.branch_list[i];
		if (v.type == NAME) {
			result += "function a:";
			str_grouped_token(result, v);
			result += "(";
		}
		else if (v.type == LAMBDA_RIGHT_SIDE) {
			const Branch &argv = v.branch_list[2];
			for (int i = 0; i < (int)argv.branch_list.size(); i++) {
				const Branch &name=argv.branch_list[i].branch_list[0];
				str_grouped_token(result, name);
				if (i != (int)argv.branch_list.size() - 1) {
					result += ", ";
				}
			}
			result += ")\n";

			const Branch &code_block = v.branch_list[4];
			code_block_to_str(result, code_block, indent + 1);

			str_indent(result, indent);
			result += "end\n";
		}
	}
}

void lambda_to_str(std::string &result, const Branch &branch,
int indent) {
	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		Branch v = branch.branch_list[i];
		if (v.type == NAME) {
			str_grouped_token(result, v);
			result += " = function(";
		}
		else if (v.type == LAMBDA_RIGHT_SIDE) {
			const Branch &argv = v.branch_list[2];
			for (int i = 0; i < (int)argv.branch_list.size(); i++) {
				const Branch &name=argv.branch_list[i].branch_list[0];
				str_grouped_token(result, name);
				if (i != (int)argv.branch_list.size() - 1) {
					result += ", ";
				}
			}
			result += ")\n";

			const Branch &code_block = v.branch_list[4];
			code_block_to_str(result, code_block, indent + 1);

			str_indent(result, indent);
			result += "end\n";
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
		else if (v.type == TYPE) {
			type_to_str(result, v, indent);
		}
		else if (v.type == RETURN) {
			return_to_str(result, v);
		}
		else if (v.type == BREAK) {
			result += "break";
		}
		else if (v.type == CONTINUE) {
			result += "continue";
		}
		else if (v.type == LAMBDA_NEW) {
			lambda_to_str(result, v, indent);
		}
		else if (v.type == LAMBDA_ASSIGN) {
			lambda_to_str(result, v, indent);
		}

		result += '\n';
	}
}
}

void tree_to_lua(std::string &result, const Branch &tree) {
	code_block_to_str(result, tree, 0);
}
