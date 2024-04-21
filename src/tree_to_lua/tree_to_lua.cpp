#include "tree_to_lua.h"

#include "../tree/types.h"
#include "../tree/calc_tree/shared.h"
#include "../tree/var_types.h"
#include "../type_checker/var_check.h"
#include <vector>

namespace {
const std::string TYPEDEFL_BEGIN = "default_";
const std::string TYPECOPY_BEGIN = "copy_";
const std::string TYPECOPY_PARAM = "copy_v";

std::vector<VarDeclare> var_declare_list;
std::vector<TypeDeclare> type_declare_list;
std::vector<int> scope_tree;

void code_block_to_str(std::string &result,
const Branch &code_block, int indent, int scope_id);

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
			Branch calc_start_content
			       = v.branch_list[0].branch_list[0];
			if (calc_start_content.type == NAME) {
				std::string var_name;
				str_grouped_token(var_name, calc_start_content);

				result += TYPECOPY_BEGIN
				+ get_var_type(var_declare_list, var_name,
				               scope_tree, -1)
				+ "(" + var_name + ")";
			} else {
				str_bracket(result, v);
			}

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

void varnew_to_str(std::string &result, const Branch &branch,
int scope_id) {
	VarDeclare declare;
	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		Branch v = branch.branch_list[i];
		if (v.type == NAME) {
			str_grouped_token(declare.var_name, v);
			result += declare.var_name;
			result += " = ";
		}
		else if (v.type == VAR_TYPE) {
			str_grouped_token(declare.var_type, v);
		}
		else if (v.type == BRACKET_ROUND) {
			Branch var_type = branch.branch_list[1];
			Branch calc_start_branch = v.branch_list[0].branch_list[0];
			if (calc_start_branch.type == BRACKET_CURLY) {
				result += TYPEDEFL_BEGIN + var_type.str;
				result += "()";
			}
			else if (calc_start_branch.type == NAME
			&& !is_primitive(var_type.str)) {
				result += TYPECOPY_BEGIN + var_type.str;
				result += "(";
				str_grouped_token(result, calc_start_branch);
				result += ")";
			}
			else {
				str_bracket(result, v);
			}
		}
	}
	declare.scope_id = scope_id;
	add_var_declare(var_declare_list, type_declare_list, declare);
}

void assign_to_str(std::string &result, const Branch &branch,
int scope_id) {
	std::string var_name;
	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		Branch v = branch.branch_list[i];
		if (v.type == NAME) {
			str_grouped_token(var_name, v);
			result += var_name;
			result += " = ";
		}
		else if (v.type == BRACKET_ROUND) {
			Branch calc_start_branch = v.branch_list[0].branch_list[0];
			std::string var_type = get_var_type(var_declare_list,
			                                    var_name,
			                                    scope_tree,
			                                    scope_id);
			if (calc_start_branch.type == NAME
			&& !is_primitive(var_type)) {
				result += TYPECOPY_BEGIN + var_type;
				result += "(";
				str_grouped_token(result, calc_start_branch);
				result += ")";
			} else {
				str_bracket(result, v);
			}
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
int indent, int scope_id) {
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
			code_block_to_str(result, v, indent + 1, scope_id);
			str_indent(result, indent);
			result += "end\n";
		}
	}
}

void if_to_str(std::string &result, const Branch &branch,
int indent, int scope_id) {
	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		Branch v = branch.branch_list[i];
		if (v.type == BRACKET_ROUND) {
			result += "if ";
			str_bracket(result, v);
			result += " then";
		}
		else if (v.type == CODE_BLOCK) {
			result += '\n';
			code_block_to_str(result, v, indent + 1, scope_id);
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
int indent, int scope_id) {
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
			code_block_to_str(result, v, indent + 1, scope_id);
			str_indent(result, indent);
			result += "end";
		}
	}
}

void for_each_to_str(std::string &result, const Branch &branch,
int indent, int scope_id) {
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
			code_block_to_str(result, v, indent + 1, scope_id);
			str_indent(result, indent);
			result += "end";
		}
	}
}

void while_to_str(std::string &result, const Branch &branch,
int indent, int scope_id) {
	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		Branch v = branch.branch_list[i];
		if (v.type == BRACKET_ROUND) {
			result += "while ";
			str_bracket(result, v);
			result += " do";
		}
		else if (v.type == CODE_BLOCK) {
			result += '\n';
			code_block_to_str(result, v, indent + 1, scope_id);
			str_indent(result, indent);
			result += "end";
		}
	}
}

void type_to_str(std::string &result, const Branch &branch,
int indent) {
	TypeDeclare declare;
	Branch name;
	Branch type_member_list;
	std::vector<std::string> param_list;
	std::vector<Branch> defl_list;

	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		Branch v = branch.branch_list[i];
		if (v.type == NAME) {
			name = v;
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
		defl_list.push_back(defl);

		VarDeclare var_declare;
		var_declare.var_name = param;
		str_grouped_token(var_declare.var_type, type);
		declare.member_list.push_back(var_declare);
	}

	result += "function " + TYPEDEFL_BEGIN;
	str_grouped_token(result, name);
	result += "()\n";
	str_indent(result, indent + 1);
	result += "return {\n";
	for (int i = 0; i < (int)param_list.size(); i++) {
		const std::string &param = param_list[i];
		const Branch &defl = defl_list[i];
		str_indent(result, indent + 2);
		result += param + " = ";
		str_bracket(result, defl);
		result += ",\n";
	}
	str_indent(result, indent + 1);
	result += "}\n";
	str_indent(result, indent);
	result += "end\n\n";

	str_indent(result, indent);
	result += "function " + TYPECOPY_BEGIN;
	str_grouped_token(result, name);
	result += "(" + TYPECOPY_PARAM + ")\n";
	str_indent(result, indent + 1);
	result += "return {\n";
	for (int i = 0; i < (int)param_list.size(); i++) {
		const std::string &param = param_list[i];
		str_indent(result, indent + 2);
		result += param + " = " + TYPECOPY_PARAM + "." + param + ",\n";
	}
	str_indent(result, indent + 1);
	result += "}\n";
	str_indent(result, indent);
	result += "end\n";


	str_grouped_token(declare.type_name, name);
	type_declare_list.push_back(declare);
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

void code_block_to_str(std::string &result,
const Branch &code_block, int indent, int scope_id) {
	int this_scope_id = (int)scope_tree.size();
	scope_tree.push_back(scope_id);

	for (int i = 0; i < (int)code_block.branch_list.size(); i++) {
		const Branch &v = code_block.branch_list[i];

		if (v.type == ELSEIF || v.type == ELSE) {
			str_indent(result, indent - 1);
		} else {
			str_indent(result, indent);
		}

		if (v.type == VARNEW) {
			varnew_to_str(result, v, this_scope_id);
		}
		else if (v.type == ASSIGN) {
			assign_to_str(result, v, this_scope_id);
		}
		else if (v.type == FUNCCALL) {
			funccall_to_str(result, v);
		}
		else if (v.type == FUNCNEW) {
			funcnew_to_str(result, v, indent, this_scope_id);
		}
		else if (v.type == IF) {
			if_to_str(result, v, indent, this_scope_id);
		}
		else if (v.type == ELSEIF) {
			elseif_to_str(result, v);
		}
		else if (v.type == ELSE) {
			result += "else";
		}
		else if (v.type == FOR) {
			for_iter_to_str(result, v, indent, this_scope_id);
		}
		else if (v.type == FOREACH) {
			for_each_to_str(result, v, indent, this_scope_id);
		}
		else if (v.type == WHILE) {
			while_to_str(result, v, indent, this_scope_id);
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

		result += '\n';
	}
}
}

void tree_to_lua(std::string &result, const Branch &tree) {
	code_block_to_str(result, tree, 0, -1);
	print_scope_tree(scope_tree);
	print_declare_lists(var_declare_list, type_declare_list);
}
