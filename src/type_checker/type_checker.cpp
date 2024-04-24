#include "type_checker.h"

#include "../tree/types.h"
#include "../err_msg/err_msg.h"
#include "var_check.h"
#include <vector>
#include <string>

#include <iostream>

namespace {
struct VarCheckLists {
	std::vector<VarDeclare> vd_list;
	std::vector<TypeDeclare> td_list;
	std::vector<FuncDeclare> fd_list;
	std::vector<int> scope_tree;
};

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

std::string get_round_bracket_value_type(const Branch &bracket,
VarCheckLists &var_check_list, int this_scope);

std::string get_funccall_bracket_value_type(const Branch &bracket,
VarCheckLists &var_check_list, int this_scope);

std::string get_calc_value_type(const Branch &calc,
VarCheckLists &var_check_list, int this_scope) {
	Branch value_token;

	if (calc.type == CALC_START) {
		value_token = calc.branch_list[0];
	}
	else {
		value_token = calc.branch_list[1];
	}

	if (value_token.type == BRACKET_ROUND) {
		return get_round_bracket_value_type(value_token,
			var_check_list, this_scope);
	}
	else if (value_token.type == BRACKET_FUNCCALL) {
		return get_funccall_bracket_value_type(value_token,
			var_check_list, this_scope);
	}

	std::string value_name;
	str_grouped_token(value_name, value_token);

	if (value_token.type == NUM) {
		return "number";
	}
	else if (value_token.type == STR) {
		return "string";
	}
	else if (value_token.type == KEYWORD
	&& (value_token.str == "true" || value_token.str == "false")) {
		return "bool";
	}
	else {
		return get_var_type(var_check_list.vd_list,
		                    value_name,
		                    var_check_list.scope_tree,
		                    this_scope);
	}
}

std::vector<std::string> num_op_list = {
	"+", "-", "*", "/", "//", "%",
};

std::vector<std::string> comp_op_list = {
	"==", "~=", ">", "<", ">=", "<=",
};

bool is_num_op(const std::string &op) {
	for (int i = 0; i < (int)num_op_list.size(); i++) {
		if (num_op_list[i] == op) {
			return true;
		}
	}
	return false;
}

bool is_comp_op(const std::string &op) {
	for (int i = 0; i < (int)comp_op_list.size(); i++) {
		if (comp_op_list[i] == op) {
			return true;
		}
	}
	return false;
}

bool is_str_op(const std::string &op) {
	if (op == "..") {
		return true;
	}
	return false;
}

bool is_logic_op(const std::string &op) {
	if (op == "and" || op == "or") {
		return true;
	}
	return false;
}

bool op_compatible(const std::string &var_type,
const std::string &op) {
	if (var_type == "number" && is_num_op(op)) {
		return true;
	}
	if (var_type == "string" && is_str_op(op)) {
		return true;
	}
	if (var_type == "number" && is_comp_op(op)) {
		return true;
	}
	if ((var_type == "string" || var_type == "bool")
	&& (op == "==" || op == "!=")) {
		return true;
	}
	if (var_type == "bool" && is_logic_op(op)) {
		return true;
	}
	return false;
}

std::string get_round_bracket_value_type(const Branch &bracket,
VarCheckLists &var_check_list, int this_scope) {
	std::string start_type;
	std::string start_op;
	for (int i = 0; i < (int)bracket.branch_list.size(); i++) {
		const Branch &v = bracket.branch_list[i];
		Branch nx;
		if (i + 1 < (int)bracket.branch_list.size()) {
			nx = bracket.branch_list[i + 1];
		}
		
		Branch value_token;
		bool is_start = false;
		if (v.type == CALC_START) {
			value_token = v.branch_list[0];
			is_start = true;
		}
		else {
			value_token = v.branch_list[1];
		}

		if (is_start) {
			start_type
				= get_calc_value_type(v, var_check_list, this_scope);
			if (nx.type != NONE) {
				start_op = nx.branch_list[0].str;
			}
		}
		else {
			Branch op_token = v.branch_list[0];
			std::string right_type = get_calc_value_type(
				v, var_check_list, this_scope);
			
			if (!op_compatible(start_type, op_token.str)
			|| !op_compatible(right_type, op_token.str)) {
				type_err_msg(op_token, INCOMPATIBLE_OP);
			}
		}
	}

	if (is_comp_op(start_op)) {
		return "bool";
	}
	return start_type;
}

std::string get_funccall_bracket_value_type(const Branch &bracket,
VarCheckLists &var_check_list, int this_scope) {
	int sz = (int)bracket.branch_list.size();
	const Branch &func_name = bracket.branch_list[sz - 1];
	
	std::vector<std::string> argv_type_list;
	get_argv_type_list(argv_type_list, func_name.str,
	                   var_check_list.vd_list,
	                   var_check_list.fd_list,
	                   this_scope);
	std::string return_type
		= get_return_type(func_name.str, var_check_list.vd_list,
		                  var_check_list.fd_list, this_scope);

	if ((int)bracket.branch_list.size() - 1
	!= (int)argv_type_list.size()) {
		type_err_msg(bracket, INCOMPATIBLE_ARGV_NUM);
		return return_type;
	}

	for (int i = 0; i < (int)bracket.branch_list.size() - 1; i++) {
		const Branch &v = bracket.branch_list[i];
		std::string v_type = get_calc_value_type(v.branch_list[0],
			var_check_list, this_scope);
		const std::string &argv_type = argv_type_list[i];

		if (v_type != argv_type) {
			type_err_msg(v, INCOMPATIBLE_TYPE);
		}
	}

	return return_type;
}

void varnew_check(const Branch &branch,
VarCheckLists &var_check_list, int this_scope) {
	Branch var_name_token;
	Branch var_type_token;
	Branch right_side;

	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		const Branch &v = branch.branch_list[i];

		if (v.type == NAME) {
			var_name_token = v;
		}
		else if (v.type == VAR_TYPE) {
			var_type_token = v;
		}
		else if (v.type == BRACKET_ROUND) {
			right_side = v;
		}
	}

	std::string right_side_type = get_round_bracket_value_type(
		right_side, var_check_list, this_scope);
	std::string var_type;
	str_grouped_token(var_type, var_type_token);
	if (right_side_type != var_type) {
		type_err_msg(right_side, INCOMPATIBLE_TYPE);
	}

	VarDeclare declare;
	str_grouped_token(declare.var_name, var_name_token);
	str_grouped_token(declare.var_type, var_type_token);
	declare.scope_id = this_scope;
	add_var_declare(var_check_list.vd_list, var_check_list.td_list,
	                declare);
}

void funcnew_check(const Branch &branch,
VarCheckLists &var_check_list, int this_scope) {
	Branch name_token;
	Branch argv_branch;
	Branch return_type_branch;
	Branch code_block;

	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		const Branch &v = branch.branch_list[i];

		if (v.type == NAME) {
			name_token = v;
		}
		else if (v.type == FUNCNEW_ARGV) {
			argv_branch = v;
		}
		else if (v.type == RETURN_TYPES) {
			return_type_branch = v.branch_list[0];
		}
		else if (v.type == CODE_BLOCK) {
			code_block = v;
		}
	}

	FuncDeclare func_declare;
	std::string func_type = "fn (";
	for (int i = 0; i < (int)argv_branch.branch_list.size(); i++) {
		const Branch &v = argv_branch.branch_list[i];
		std::string type_str;
		str_grouped_token(type_str, v.branch_list[1]);

		func_declare.argv.push_back(type_str);

		func_type += type_str;
		if (i != (int)argv_branch.branch_list.size() - 1) {
			func_type += ",";
		}
	}
	func_type += ") -> ";
	str_grouped_token(func_type, return_type_branch);

	VarDeclare var_declare;
	str_grouped_token(var_declare.var_name, name_token);
	var_declare.var_type = func_type;
	var_declare.scope_id = this_scope;
	add_var_declare(var_check_list.vd_list,
	                var_check_list.td_list,
	                var_declare);

	func_declare.var_declare_index
		= (int)var_check_list.vd_list.size() - 1;
	str_grouped_token(func_declare.return_type, return_type_branch);
	var_check_list.fd_list.push_back(func_declare);
}

void code_block_check(const Branch &code_block,
VarCheckLists &var_check_list, int parent_scope) {
	int this_scope = (int)var_check_list.scope_tree.size();

	for (int i = 0; i < (int)code_block.branch_list.size(); i++) {
		const Branch &v = code_block.branch_list[i];
		
		if (v.type == VARNEW) {
			varnew_check(v, var_check_list, this_scope);
		}
		else if (v.type == FUNCNEW) {
			funcnew_check(v, var_check_list, this_scope);
		}
	}
}
}

void type_check(const Branch &tree) {
	VarCheckLists var_check_list;
	code_block_check(tree, var_check_list, -1);
	print_declare_lists(var_check_list.vd_list,
	                    var_check_list.td_list);
	std::cout << std::endl;
}
