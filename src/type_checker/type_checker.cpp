#include "type_checker.h"

#include "../tree/types.h"
#include "../err_msg/err_msg.h"
#include "var_check.h"
#include "../tree/var_types.h"
#include "../lib_func/lib_func.h"
#include <vector>
#include <string>

#include <iostream>

namespace {
enum CodeBlockType: int {
	BLOCK_FUNC,
	BLOCK_LOOP,
	BLOCK_IF,
};

std::vector<std::string> code_block_return_type_list;

struct VarCheckLists {
	std::vector<VarDeclare> vd_list;
	std::vector<TypeDeclare> td_list;
	std::vector<FuncDeclare> fd_list;
	std::vector<int> scope_tree;
};

std::string get_nearest_return_type(
const VarCheckLists &var_check_lists, int this_scope) {
	while (true) {
		std::string return_type
		            = code_block_return_type_list[this_scope];
		int parent_scope = var_check_lists.scope_tree[this_scope];

		if (return_type == "is BLOCK_IF"
		|| return_type == "is BLOCK_LOOP") {
			this_scope = parent_scope;
		}
		else {
			return return_type;
		}
	}
	return "";
}

bool can_break(
const VarCheckLists &var_check_lists, int this_scope) {
	while (true) {
		std::string return_type
		            = code_block_return_type_list[this_scope];
		int parent_scope = var_check_lists.scope_tree[this_scope];

		if (return_type == "is BLOCK_IF") {
			this_scope = parent_scope;
		}
		else if (return_type == "is BLOCK_LOOP") {
			return true;
		}
		else {
			return false;
		}
	}
	return false;
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

std::string get_round_bracket_value_type(const Branch &bracket,
VarCheckLists &var_check_lists, int this_scope);

std::string get_funccall_bracket_value_type(const Branch &bracket,
VarCheckLists &var_check_lists, int this_scope);

std::string get_calc_value_type(const Branch &calc,
VarCheckLists &var_check_lists, int this_scope) {
	Branch value_token;

	if (calc.type == CALC_START) {
		value_token = calc.branch_list[0];
	}
	else {
		value_token = calc.branch_list[1];
	}

	if (value_token.type == BRACKET_ROUND) {
		return get_round_bracket_value_type(value_token,
			var_check_lists, this_scope);
	}
	else if (value_token.type == BRACKET_FUNCCALL) {
		return get_funccall_bracket_value_type(value_token,
			var_check_lists, this_scope);
	}
	else if (value_token.type == REVERSED) {
		std::string type = get_calc_value_type(
			value_token.branch_list[0],
		    var_check_lists, this_scope);
		std::string reversed_str = value_token.branch_list[1].str;
		if (reversed_str == "-" && type != "number") {
			type_err_msg(value_token.branch_list[0],
				INCOMPATIBLE_TYPE,"number",type);
		}
		if (reversed_str=="not" && type != "bool") {
			type_err_msg(value_token.branch_list[0],
				INCOMPATIBLE_TYPE, "bool",type);
		}
		return type;
	}
	else if (value_token.type == BRACKET_SQUARE) {
		return "[]";
	}
	else if (value_token.type == BRACKET_CURLY) {
		return "{}";
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
		std::string var_type = get_var_type(var_check_lists.vd_list,
		                       value_name,
		                       var_check_lists.scope_tree,
		                       this_scope);
		if (var_type == "") {
			type_err_msg(value_token, VAR_NOT_DECLARED, "", "");
		}
		return var_type;
	}
	return "";
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
VarCheckLists &var_check_lists, int this_scope) {
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
				= get_calc_value_type(v, var_check_lists, this_scope);
			if (nx.type != NONE) {
				start_op = nx.branch_list[0].str;
			}
		}
		else {
			Branch op_token = v.branch_list[0];
			std::string right_type = get_calc_value_type(
				v, var_check_lists, this_scope);
			
			if (!op_compatible(start_type, op_token.str)
			|| !op_compatible(right_type, op_token.str)) {
				type_err_msg(op_token, INCOMPATIBLE_OP, "", "");
			}
		}
	}

	if (is_comp_op(start_op)) {
		return "bool";
	}
	return start_type;
}

std::string get_funccall_bracket_value_type(const Branch &bracket,
VarCheckLists &var_check_lists, int this_scope) {
	int sz = (int)bracket.branch_list.size();
	const Branch &func_name = bracket.branch_list[sz - 1];
	
	std::string func_name_str;
	str_grouped_token(func_name_str, func_name);

	std::string lib_func_return_type
		= get_lib_func_return_type(func_name_str);
	if (lib_func_return_type != "") {
		return lib_func_return_type;
	}

	std::string return_type
		= get_return_type(func_name_str, var_check_lists.vd_list,
		                  var_check_lists.fd_list,
		                  var_check_lists.scope_tree,
		                  this_scope);

	if (return_type == "") {
		type_err_msg(func_name, FUNC_NOT_DECLARED, "", "");
		return "";
	}

	std::vector<std::string> argv_type_list;
	get_argv_type_list(argv_type_list, func_name_str,
	                   var_check_lists.vd_list,
	                   var_check_lists.fd_list,
	                   var_check_lists.scope_tree,
	                   this_scope);

	if ((int)bracket.branch_list.size() - 1
	!= (int)argv_type_list.size()) {
		type_err_msg(bracket, INCOMPATIBLE_ARGV_NUM, "", "");
		return return_type;
	}

	for (int i = 0; i < (int)bracket.branch_list.size() - 1; i++) {
		const Branch &v = bracket.branch_list[i];
		std::string v_type = get_calc_value_type(v.branch_list[0],
			var_check_lists, this_scope);
		const std::string &argv_type = argv_type_list[i];

		if(!type_compatible(var_check_lists.td_list,argv_type,v_type)){
			type_err_msg(v, INCOMPATIBLE_TYPE, argv_type, v_type);
		}
	}

	return return_type;
}

std::string get_array_type(const Branch &branch,
VarCheckLists &var_check_lists, int this_scope) {
	std::string start_type;
	const Branch bracket = branch.branch_list[0].branch_list[0];
	for (int i = 0; i < (int)bracket.branch_list.size(); i++) {
		const Branch &v = bracket.branch_list[i];
		const Branch &arg = v.branch_list[0];
		std::string type = get_calc_value_type(
			arg, var_check_lists, this_scope);

		if (i == 0) {
			start_type = type;
		}
		else {
			if (start_type != type) {
				type_err_msg(v, INCOMPATIBLE_TYPE,
					start_type, type);
			}
		}
	}
	return start_type + "[]";
}

void varnew_check(const Branch &branch,
VarCheckLists &var_check_lists, int this_scope) {
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

	std::string var_name;
	str_grouped_token(var_name, var_name_token);
	std::string var_type_test = get_var_type(var_check_lists.vd_list,
	                              var_name, var_check_lists.scope_tree,
	                              this_scope);
	if (var_type_test != "") {
		type_err_msg(var_name_token, VAR_ALREADY_DECLARED, "", "");
		return;
	}

	if (!is_primitive(var_type_token.str)) {
		int type_index = get_type_declare_i(var_check_lists.td_list,
		                                    var_type_token.str);
		if (type_index == -1) {
			type_err_msg(var_type_token, TYPE_NOT_DECLARED, "", "");
		}
	}

	std::string var_type;
	str_grouped_token(var_type, var_type_token);
	std::string check_type;
	bool is_array = false;

	const Branch &first_calc = right_side.branch_list[0]
	                                     .branch_list[0];
	if (first_calc.type == BRACKET_SQUARE) {
		is_array = true;
	}

	if (is_array)
	{
		std::string array_type
			= get_array_type(right_side, var_check_lists, this_scope);
		check_type = array_type;
	}
	else {
		std::string right_side_type = get_round_bracket_value_type(
			right_side, var_check_lists, this_scope);
		check_type = right_side_type;
	}

	if (is_array && var_type[var_type.length()-1] == ']'
	&& check_type == "[]") {
		check_type = var_type;
	}
	if (!type_compatible(var_check_lists.td_list,var_type,check_type)){
		if (is_primitive(var_type) || is_array) {
			type_err_msg(right_side,INCOMPATIBLE_TYPE,
			             var_type,check_type);
		}
		else {
			if (check_type != "{}") {
				type_err_msg(right_side,INCOMPATIBLE_TYPE,
							 var_type,check_type);
			}
		}
	}

	VarDeclare declare;
	declare.var_name = var_name;
	str_grouped_token(declare.var_type, var_type_token);
	declare.scope_id = this_scope;
	add_var_declare(var_check_lists.vd_list, var_check_lists.td_list,
	                var_check_lists.fd_list, declare);
}

void assign_check(const Branch &branch,
VarCheckLists &var_check_lists, int this_scope) {
	Branch var_name_token;
	Branch right_side;

	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		const Branch &v = branch.branch_list[i];

		if (v.type == NAME) {
			var_name_token = v;
		}
		else if (v.type == BRACKET_ROUND) {
			right_side = v;
		}
	}

	std::string var_name;
	str_grouped_token(var_name, var_name_token);
	std::string var_type = get_var_type(
		var_check_lists.vd_list,
		var_name,
		var_check_lists.scope_tree,
		this_scope);

	if (var_type == "") {
		type_err_msg(var_name_token, VAR_NOT_DECLARED, "", "");
		return;
	}

	bool is_arr = false;
	std::string check_type;

	const Branch &first_calc = right_side.branch_list[0]
	                                     .branch_list[0];
	if (first_calc.type == BRACKET_SQUARE) {
		is_arr = true;
	}

	if (is_arr) {
		std::string array_type
			= get_array_type(right_side, var_check_lists, this_scope);
		check_type = array_type;
	}

	else {
		std::string right_side_type = get_round_bracket_value_type(
			right_side, var_check_lists, this_scope);
		check_type = right_side_type;
	}

	if (is_arr && var_type[var_type.length()-1] == ']'
	&& check_type == "[]") {
		check_type = var_type;
	}
	if (!type_compatible(var_check_lists.td_list,var_type,check_type)){
		type_err_msg(right_side,INCOMPATIBLE_TYPE,var_type,check_type);
	}
}

void funcnew_add(const Branch &branch,
VarCheckLists &var_check_lists, int this_scope) {
	Branch name_token;
	Branch argv_branch;
	Branch return_type_branch;

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
	}

	std::string var_name;
	str_grouped_token(var_name, name_token);
	std::string var_type_test = get_var_type(var_check_lists.vd_list,
	                              var_name, var_check_lists.scope_tree,
	                              this_scope);
	
	if (var_type_test != "") {
		type_err_msg(name_token, FUNC_ALREADY_DECLARED, "", "");
		return;
	}

//	FuncDeclare func_declare;
	std::string func_type = "fn (";
	for (int i = 0; i < (int)argv_branch.branch_list.size(); i++) {
		const Branch &v = argv_branch.branch_list[i];
		std::string type_str;
		str_grouped_token(type_str, v.branch_list[1]);

//		func_declare.argv.push_back(type_str);

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
	add_var_declare(var_check_lists.vd_list,
	                var_check_lists.td_list,
	                var_check_lists.fd_list,
	                var_declare);

//	func_declare.var_declare_index
//		= (int)var_check_lists.vd_list.size() - 1;
//	str_grouped_token(func_declare.return_type, return_type_branch);
//	var_check_lists.fd_list.push_back(func_declare);
}

void code_block_check(const Branch &code_block,
VarCheckLists &var_check_lists, int parent_scope,
CodeBlockType code_block_type,
const std::string &return_type);

void funcnew_check(const Branch &branch,
VarCheckLists &var_check_lists, int this_scope) {
	if (this_scope != 0) {
		type_err_msg(branch, FUNCNEW_ONLY_IN_ROOT_SCOPE, "", "");
	}

	Branch argv_branch;
	Branch return_type_branch;
	Branch code_block;

	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		const Branch &v = branch.branch_list[i];
		
		if (v.type == FUNCNEW_ARGV) {
			argv_branch = v;
		}
		else if (v.type == RETURN_TYPES) {
			return_type_branch = v.branch_list[0];
		}
		else if (v.type == CODE_BLOCK) {
			code_block = v;
		}
	}

	for (int i = 0; i < (int)argv_branch.branch_list.size(); i++) {
		const Branch &v = argv_branch.branch_list[i];
		const Branch &var_name = v.branch_list[0];
		const Branch &var_type = v.branch_list[1];
		
		if (!is_primitive(var_type.str)) {
			int index = get_type_declare_i(var_check_lists.td_list,
										   var_type.str);
			if (index == -1) {
				type_err_msg(var_type, TYPE_NOT_DECLARED, "", "");
			}
		}

		std::string var_name_str;
		str_grouped_token(var_name_str, var_name);
		std::string search_type = get_var_type(
			var_check_lists.vd_list, var_name_str,
			var_check_lists.scope_tree, this_scope);
		if (search_type != "") {
			type_err_msg(v, VAR_ALREADY_DECLARED, "", "");
		}

		VarDeclare var_declare;
		var_declare.var_name = var_name_str;
		str_grouped_token(var_declare.var_type, var_type);
		int nx_scope_id = (int)var_check_lists.scope_tree.size();
		var_declare.scope_id = nx_scope_id;
		var_declare.branch = v;
		add_var_declare(
			var_check_lists.vd_list, var_check_lists.td_list,
			var_check_lists.fd_list, var_declare);
	}

	if (!is_primitive(return_type_branch.str)
	&& return_type_branch.str != "void") {
		int index = get_type_declare_i(var_check_lists.td_list,
									   return_type_branch.str);
		if (index == -1) {
			type_err_msg(return_type_branch,TYPE_NOT_DECLARED, "", "");
		}
	}

	std::string return_type;
	str_grouped_token(return_type, return_type_branch);

	code_block_check(code_block, var_check_lists, this_scope,
	                 BLOCK_FUNC, return_type);

	if (return_type == "void") {
		return;
	}

	int cb_sz = (int)code_block.branch_list.size();
	if (cb_sz == 0) {
		type_err_msg(code_block, MUST_RETURN_END_OF_FUNC, "", "");
		return;
	}
	const Branch &last_command = code_block.branch_list[cb_sz - 1];
	if (last_command.type != RETURN) {
		type_err_msg(last_command, MUST_RETURN_END_OF_FUNC, "", "");
	}
}

void typenew_member_check(const Branch &right_side,
VarCheckLists &var_check_lists, int this_scope,
const std::string &type) {
	std::string check_type;
	bool is_array = false;
	const Branch &first_calc = right_side.branch_list[0]
										 .branch_list[0];
	if (first_calc.type == BRACKET_SQUARE) {
		is_array = true;
	}

	if (is_array)
	{
		std::string array_type
		= get_array_type(right_side, var_check_lists, this_scope);

		check_type = array_type;
	}
	else {
		std::string right_side_type = get_round_bracket_value_type(
			right_side, var_check_lists, this_scope);
		check_type = right_side_type;
	}

	if (!type_compatible(var_check_lists.td_list,type,check_type)){
		if (is_primitive(type) || is_array) {
			type_err_msg(right_side,INCOMPATIBLE_TYPE,
						 type,check_type);
		}
		else {
			if (check_type != "{}") {
				type_err_msg(right_side,INCOMPATIBLE_TYPE,
							 type,check_type);
			}
		}
	}
}

void typenew_lambda_add_argv(const Branch &argv,
VarCheckLists &var_check_lists) {
	for (int i = 0; i < (int)argv.branch_list.size(); i++) {
		const Branch &v = argv.branch_list[i];
		const Branch &name_branch = v.branch_list[0];
		const Branch &type_branch = v.branch_list[1];

		VarDeclare vd;
		str_grouped_token(vd.var_name, name_branch);
		str_grouped_token(vd.var_type, type_branch);
		vd.scope_id = (int)var_check_lists.scope_tree.size();
		vd.branch = v;

		add_var_declare(
			var_check_lists.vd_list,
			var_check_lists.td_list,
			var_check_lists.fd_list,
			vd);
	}
}

void typenew_check(const Branch &branch,
VarCheckLists &var_check_lists, int this_scope) {
	std::string name;
	std::string inherit;
	Branch type_member_list;

	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		const Branch &v = branch.branch_list[i];
		if (i == 0) {
			str_grouped_token(name, v);
		}
		else if (i == 1) {
			if (v.type != NONE) {
				str_grouped_token(inherit, v);
			}
		}
		else if (v.type == TYPE_MEMBER_LIST) {
			type_member_list = v;
		}
	}

	int test_index = get_type_declare_i(var_check_lists.td_list,
	                                    name);
	if (test_index != -1) {
		type_err_msg(branch.branch_list[0],
		             TYPE_ALREADY_DECLARED,"","");
		return;
	}

	TypeDeclare type_declare;
	type_declare.type_name = name;
	type_declare.inherit = inherit;
	type_declare.inherit_branch = branch.branch_list[1];

	for (int i = 0; i < (int)type_member_list.branch_list.size(); i++){
		const Branch &v = type_member_list.branch_list[i];
		std::string type;
		str_grouped_token(type, v.branch_list[1]);

		const Branch right_side = v.branch_list[3];

		if (type != "fn") {
			typenew_member_check(right_side, var_check_lists,
			                     this_scope, type);

			VarDeclare var_declare;
			str_grouped_token(var_declare.var_name, v.branch_list[0]);
			var_declare.var_type = type;
			var_declare.branch = v.branch_list[1];
			type_declare.member_list.push_back(var_declare);
		}
		else {
			std::string check_type = v.branch_list[4].str;

			VarDeclare var_declare;
			str_grouped_token(var_declare.var_name, v.branch_list[0]);
			var_declare.var_type = check_type;
			var_declare.branch = v.branch_list[1];
			type_declare.member_list.push_back(var_declare);
		}
	}

	var_check_lists.td_list.push_back(type_declare);

	for (int i = 0; i < (int)type_member_list.branch_list.size(); i++){
		const Branch &v = type_member_list.branch_list[i];
		std::string type;
		str_grouped_token(type, v.branch_list[1]);

		const Branch right_side = v.branch_list[3];

		if (type == "fn") {
			VarDeclare self_vd;
			self_vd.var_name = "self";
			self_vd.var_type = name;
			self_vd.scope_id = var_check_lists.scope_tree.size();
			add_var_declare(var_check_lists.vd_list,
				var_check_lists.td_list,
				var_check_lists.fd_list,
				self_vd);

			const Branch &argv_branch
			             = right_side.branch_list[2];
			typenew_lambda_add_argv(argv_branch, var_check_lists);

			std::string return_type;
			const Branch &return_type_branch
			             = right_side.branch_list[3].branch_list[0];

			str_grouped_token(return_type, return_type_branch);
				const Branch &code_block = right_side.branch_list[4];
			code_block_check(code_block, var_check_lists, this_scope,
			                 BLOCK_FUNC, return_type);
		}
	}
}

void funccall_check(const Branch &branch,
VarCheckLists &var_check_lists, int this_scope) {
	Branch bracket = branch.branch_list[2];
	get_funccall_bracket_value_type(bracket,
		var_check_lists, this_scope);
}

void lambda_new_check(const Branch &branch,
VarCheckLists &var_check_lists, int this_scope) {
	Branch var_type_branch;
	Branch name_token;
	Branch argv_branch;
	Branch return_type_branch;
	Branch code_block;
	
	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		const Branch &v = branch.branch_list[i];
		if (v.type == NAME) {
			name_token = v;
		}
		else if (v.type == VAR_TYPE) {
			var_type_branch = v;
		}
		else if (v.type == LAMBDA_RIGHT_SIDE) {
			for (int j = 0; j < (int)v.branch_list.size(); j++) {
				const Branch &vv = v.branch_list[j];
				if (vv.type == FUNCNEW_ARGV) {
					argv_branch = vv;
				}
				else if (vv.type == RETURN_TYPES) {
					return_type_branch = vv.branch_list[0];
				}
				else if (vv.type == CODE_BLOCK) {
					code_block = vv;
				}
			}
		}
	}

	std::string var_name;
	str_grouped_token(var_name, name_token);
	std::string var_type_test = get_var_type(var_check_lists.vd_list,
	                              var_name, var_check_lists.scope_tree,
	                              this_scope);
	
	if (var_type_test != "") {
		type_err_msg(name_token, FUNC_ALREADY_DECLARED, "", "");
		return;
	}

	FuncDeclare func_declare;
	for (int i = 0; i < (int)argv_branch.branch_list.size(); i++) {
		const Branch &v = argv_branch.branch_list[i];
		std::string type_str;
		str_grouped_token(type_str, v.branch_list[1]);

		func_declare.argv.push_back(type_str);
	}

	VarDeclare var_declare;
	str_grouped_token(var_declare.var_name, name_token);
	str_grouped_token(var_declare.var_type, var_type_branch);
	var_declare.scope_id = this_scope;
	add_var_declare(var_check_lists.vd_list,
	                var_check_lists.td_list,
	                var_check_lists.fd_list,
	                var_declare);

	func_declare.var_declare_index
		= (int)var_check_lists.vd_list.size() - 1;
	str_grouped_token(func_declare.return_type, return_type_branch);
	var_check_lists.fd_list.push_back(func_declare);


	for (int i = 0; i < (int)argv_branch.branch_list.size(); i++) {
		const Branch &v = argv_branch.branch_list[i];
		const Branch &var_name = v.branch_list[0];
		const Branch &var_type = v.branch_list[1];
		
		if (!is_primitive(var_type.str)) {
			int index = get_type_declare_i(var_check_lists.td_list,
										   var_type.str);
			if (index == -1) {
				type_err_msg(var_type, TYPE_NOT_DECLARED, "", "");
			}
		}

		std::string var_name_str;
		str_grouped_token(var_name_str, var_name);
		std::string search_type = get_var_type(
			var_check_lists.vd_list, var_name_str,
			var_check_lists.scope_tree, this_scope);
		if (search_type != "") {
			type_err_msg(v, VAR_ALREADY_DECLARED, "", "");
		}

		VarDeclare var_declare;
		var_declare.var_name = var_name_str;
		str_grouped_token(var_declare.var_type, var_type);
		int nx_scope_id = (int)var_check_lists.scope_tree.size();
		var_declare.scope_id = nx_scope_id;
		var_declare.branch = v;
		add_var_declare(
			var_check_lists.vd_list, var_check_lists.td_list,
			var_check_lists.fd_list, var_declare);
	}

	if (!is_primitive(return_type_branch.str)
	&& return_type_branch.str != "void") {
		int index = get_type_declare_i(var_check_lists.td_list,
									   return_type_branch.str);
		if (index == -1) {
			type_err_msg(return_type_branch,TYPE_NOT_DECLARED, "", "");
		}
	}

	std::string return_type;
	str_grouped_token(return_type, return_type_branch);

	code_block_check(code_block, var_check_lists, this_scope,
	                 BLOCK_FUNC, return_type);

	if (return_type == "void") {
		return;
	}

	int cb_sz = (int)code_block.branch_list.size();
	if (cb_sz == 0) {
		type_err_msg(code_block, MUST_RETURN_END_OF_FUNC, "", "");
		return;
	}
	const Branch &last_command = code_block.branch_list[cb_sz - 1];
	if (last_command.type != RETURN) {
		type_err_msg(last_command, MUST_RETURN_END_OF_FUNC, "", "");
	}
}

void lambda_assign_check(const Branch &branch,
VarCheckLists &var_check_lists, int this_scope) {
	Branch var_type_branch;
	Branch name_token;
	Branch argv_branch;
	Branch return_type_branch;
	Branch code_block;
	
	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		const Branch &v = branch.branch_list[i];
		if (v.type == NAME) {
			name_token = v;
		}
		else if (v.type == VAR_TYPE) {
			var_type_branch = v;
		}
		else if (v.type == LAMBDA_RIGHT_SIDE) {
			for (int j = 0; j < (int)v.branch_list.size(); j++) {
				const Branch &vv = v.branch_list[j];
				if (vv.type == FUNCNEW_ARGV) {
					argv_branch = vv;
				}
				else if (vv.type == RETURN_TYPES) {
					return_type_branch = vv.branch_list[0];
				}
				else if (vv.type == CODE_BLOCK) {
					code_block = vv;
				}
			}
		}
	}

	for (int i = 0; i < (int)argv_branch.branch_list.size(); i++) {
		const Branch &v = argv_branch.branch_list[i];
		const Branch &var_name = v.branch_list[0];
		const Branch &var_type = v.branch_list[1];
		
		if (!is_primitive(var_type.str)) {
			int index = get_type_declare_i(var_check_lists.td_list,
										   var_type.str);
			if (index == -1) {
				type_err_msg(var_type, TYPE_NOT_DECLARED, "", "");
			}
		}

		std::string var_name_str;
		str_grouped_token(var_name_str, var_name);
		std::string search_type = get_var_type(
			var_check_lists.vd_list, var_name_str,
			var_check_lists.scope_tree, this_scope);
		if (search_type != "") {
			type_err_msg(v, VAR_ALREADY_DECLARED, "", "");
		}

		VarDeclare var_declare;
		var_declare.var_name = var_name_str;
		str_grouped_token(var_declare.var_type, var_type);
		int nx_scope_id = (int)var_check_lists.scope_tree.size();
		var_declare.scope_id = nx_scope_id;
		var_declare.branch = v;
		add_var_declare(
			var_check_lists.vd_list, var_check_lists.td_list,
			var_check_lists.fd_list, var_declare);
	}

	std::string type;
	str_grouped_token(type, var_type_branch);
	std::string name;
	str_grouped_token(name, name_token);
	std::string searched_type = get_var_type(
		var_check_lists.vd_list, name,
		var_check_lists.scope_tree, this_scope);
	if (searched_type != type) {
		type_err_msg(name_token, INCOMPATIBLE_TYPE,searched_type,type);
	}

	std::string return_type;
	str_grouped_token(return_type, return_type_branch);

	code_block_check(code_block, var_check_lists, this_scope,
	                 BLOCK_FUNC, return_type);

	if (return_type == "void") {
		return;
	}

	int cb_sz = (int)code_block.branch_list.size();
	if (cb_sz == 0) {
		type_err_msg(code_block, MUST_RETURN_END_OF_FUNC, "", "");
		return;
	}
	const Branch &last_command = code_block.branch_list[cb_sz - 1];
	if (last_command.type != RETURN) {
		type_err_msg(last_command, MUST_RETURN_END_OF_FUNC, "", "");
	}
}

void if_check(const Branch &branch,
VarCheckLists &var_check_lists, int this_scope) {
	Branch cond;
	Branch code_block;

	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		const Branch &v = branch.branch_list[i];

		if (v.type == BRACKET_ROUND) {
			cond = v;
		}
		else if (v.type == CODE_BLOCK) {
			code_block = v;
		}
	}

	std::string cond_type = get_round_bracket_value_type(
		cond, var_check_lists, this_scope);
	if (cond_type != "bool") {
		type_err_msg(cond, INCOMPATIBLE_TYPE, "bool", cond_type);
	}

	code_block_check(code_block, var_check_lists, this_scope,
	                 BLOCK_IF, "");
}

void elseif_check(const Branch &branch,
VarCheckLists &var_check_lists, int this_scope) {
	Branch cond;

	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		const Branch &v = branch.branch_list[i];

		if (v.type == BRACKET_ROUND) {
			cond = v;
		}
	}

	std::string cond_type = get_round_bracket_value_type(
		cond, var_check_lists, this_scope);
	if (cond_type != "bool") {
		type_err_msg(cond, INCOMPATIBLE_TYPE, "bool", cond_type);
	}
}

void for_check(const Branch &branch,
VarCheckLists &var_check_lists, int this_scope) {
	Branch iter_name_token;
	Branch funccall_token;
	Branch code_block;

	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		const Branch &v = branch.branch_list[i];

		if (v.type == NAME) {
			iter_name_token = v;
		}
		else if (v.type == BRACKET_FUNCCALL) {
			funccall_token = v;
		}
		else if (v.type == CODE_BLOCK) {
			code_block = v;
		}
	}

	VarDeclare var_declare;
	str_grouped_token(var_declare.var_name, iter_name_token);
	var_declare.var_type = "number";
	var_declare.scope_id = (int)var_check_lists.scope_tree.size();
	var_declare.branch = iter_name_token;
	add_var_declare(
		var_check_lists.vd_list,
		var_check_lists.td_list,
		var_check_lists.fd_list,
		var_declare);

	std::string iter_name;
	str_grouped_token(iter_name, iter_name_token);
	std::string iter_type = get_var_type(var_check_lists.vd_list,
		iter_name, var_check_lists.scope_tree, this_scope);
	if (iter_type != "") {
		type_err_msg(iter_name_token, VAR_ALREADY_DECLARED, "", "");
	}

	for (int i = 0; i < (int)funccall_token.branch_list.size()-1;i++) {
		const Branch &v = funccall_token.branch_list[i];
		const Branch &calc = v.branch_list[0];

		std::string calc_type = get_calc_value_type(calc,
			var_check_lists, this_scope);

		if (calc_type != "number") {
			type_err_msg(v, INCOMPATIBLE_TYPE, "number", calc_type);
		}
	}

	code_block_check(code_block, var_check_lists, this_scope,
		BLOCK_LOOP, "");
}

void foreach_check(const Branch &branch,
VarCheckLists &var_check_lists, int this_scope) {
	Branch iter_token;
	Branch val_token;
	Branch list_token;
	Branch code_block;

	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		const Branch &v = branch.branch_list[i];
		if (i == 0) {
			iter_token = v;
		}
		else if (i == 1) {
			val_token = v;
		}
		else if (i == 2) {
			list_token = v;
		}
		else if (v.type == CODE_BLOCK) {
			code_block = v;
		}
	}

	std::string iter_name;
	str_grouped_token(iter_name, iter_token);
	std::string val_name;
	str_grouped_token(val_name, val_token);
	std::string list_name;
	str_grouped_token(list_name, list_token);

	std::string iter_type = get_var_type(var_check_lists.vd_list,
		iter_name, var_check_lists.scope_tree, this_scope);
	std::string val_type = get_var_type(var_check_lists.vd_list,
		val_name, var_check_lists.scope_tree, this_scope);
	std::string list_type = get_var_type(var_check_lists.vd_list,
		list_name, var_check_lists.scope_tree, this_scope);

	VarDeclare iter_var_declare;
	iter_var_declare.var_name = iter_name;
	iter_var_declare.var_type = "number";
	iter_var_declare.scope_id = (int)var_check_lists.scope_tree.size();
	iter_var_declare.branch = iter_token;
	add_var_declare(
		var_check_lists.vd_list,
		var_check_lists.td_list,
		var_check_lists.fd_list,
		iter_var_declare);

	if (list_type[list_type.size() - 1] == ']') {
		VarDeclare val_var_declare;
		val_var_declare.var_name = val_name;
		val_var_declare.var_type
			= list_type.substr(0,list_type.length()-2);
		val_var_declare.scope_id
			= (int)var_check_lists.scope_tree.size();
		val_var_declare.branch = val_token;
		add_var_declare(
			var_check_lists.vd_list,
			var_check_lists.td_list,
			var_check_lists.fd_list,
			val_var_declare);
	}

	if (iter_type != "") {
		type_err_msg(iter_token, VAR_ALREADY_DECLARED, "", "");
	}
	if (val_type != "") {
		type_err_msg(val_token, VAR_ALREADY_DECLARED, "", "");
	}
	if (list_type == "") {
		type_err_msg(list_token, VAR_NOT_DECLARED, "", "");
	}
	if (list_type[list_type.size() - 1] != ']') {
		type_err_msg(list_token,INCOMPATIBLE_TYPE,"array[]",list_type);
	}

	code_block_check(code_block, var_check_lists, this_scope,
		BLOCK_LOOP, "");
}

void while_check(const Branch &branch,
VarCheckLists &var_check_lists, int this_scope) {
	Branch cond;
	Branch code_block;

	for (int i = 0; i < (int)branch.branch_list.size(); i++) {
		const Branch &v = branch.branch_list[i];

		if (v.type == BRACKET_ROUND) {
			cond = v;
		}
		else if (v.type == CODE_BLOCK) {
			code_block = v;
		}
	}

	std::string cond_type = get_round_bracket_value_type(
		cond, var_check_lists, this_scope);
	if (cond_type != "bool") {
		type_err_msg(cond, INCOMPATIBLE_TYPE, "bool", cond_type);
	}

	code_block_check(code_block, var_check_lists, this_scope,
	                 BLOCK_LOOP, "");
}

void return_check(const Branch &branch,
VarCheckLists &var_check_lists, int this_scope) {
	std::string nearest_return_type
		= get_nearest_return_type(var_check_lists, this_scope);

	std::string right_side_type;
	Branch right_side;
	if (branch.branch_list.size() == 0) {
		right_side_type = "void";
		right_side = branch;
	}
	else {
		right_side = branch.branch_list[0];
		right_side_type
			= get_round_bracket_value_type(right_side, var_check_lists,
										   this_scope);
	}

	if (nearest_return_type != right_side_type) {
		type_err_msg(right_side, INCOMPATIBLE_TYPE,
		             nearest_return_type, right_side_type);
	}
}

void break_check(const Branch &branch,
VarCheckLists &var_check_lists, int this_scope) {
	if (this_scope == 0) {
		type_err_msg(branch, CANT_BREAK, "", "");
		return;
	}
	if (!can_break(var_check_lists, this_scope)) {
		type_err_msg(branch, CANT_BREAK, "", "");
	}
}

void code_block_check(const Branch &code_block,
VarCheckLists &var_check_lists, int parent_scope,
CodeBlockType code_block_type,
const std::string &return_type) {
	int this_scope = (int)var_check_lists.scope_tree.size();
	var_check_lists.scope_tree.push_back(parent_scope);
	if (code_block_type == BLOCK_IF) {
		code_block_return_type_list.push_back("is BLOCK_IF");
	}
	else if (code_block_type == BLOCK_LOOP) {
		code_block_return_type_list.push_back("is BLOCK_LOOP");
	}
	else {
		code_block_return_type_list.push_back(return_type);
	}

	for (int i = 0; i < (int)code_block.branch_list.size(); i++) {
		const Branch &v = code_block.branch_list[i];
		if (v.type == FUNCNEW) {
			funcnew_add(v, var_check_lists, this_scope);
		}
	}

	for (int i = 0; i < (int)code_block.branch_list.size(); i++) {
		const Branch &v = code_block.branch_list[i];
		
		if (v.type == VARNEW) {
			varnew_check(v, var_check_lists, this_scope);
		}
		else if (v.type == ASSIGN) {
			assign_check(v, var_check_lists, this_scope);
		}
		else if (v.type == FUNCNEW) {
			funcnew_check(v, var_check_lists, this_scope);
		}
		else if (v.type == TYPE) {
			typenew_check(v, var_check_lists, this_scope);
		}
		else if (v.type == FUNCCALL) {
			funccall_check(v, var_check_lists, this_scope);
		}
		else if (v.type == LAMBDA_NEW) {
			lambda_new_check(v, var_check_lists, this_scope);
		}
		else if (v.type == LAMBDA_ASSIGN) {
			lambda_assign_check(v, var_check_lists, this_scope);
		}
		else if (v.type == IF) {
			if_check(v, var_check_lists, this_scope);
		}
		else if (v.type == ELSEIF) {
			elseif_check(v, var_check_lists, this_scope);
		}
		else if (v.type == FOR) {
			for_check(v, var_check_lists, this_scope);
		}
		else if (v.type == FOREACH) {
			foreach_check(v, var_check_lists, this_scope);
		}
		else if (v.type == WHILE) {
			while_check(v, var_check_lists, this_scope);
		}

		else if (v.type == RETURN) {
			return_check(v, var_check_lists, this_scope);
		}
		else if (v.type == CONTINUE) {
			type_err_msg(v, CONTINUE_UNSUPPORTED, "", "");
		}
		else if (v.type == BREAK) {
			break_check(v, var_check_lists, this_scope);
		}
	}
}
}

void type_check(const Branch &tree) {
	code_block_return_type_list.clear();

	VarCheckLists var_check_lists;
	code_block_check(tree, var_check_lists, -1, BLOCK_FUNC, "void");
	print_declare_lists(var_check_lists.vd_list,
	                    var_check_lists.td_list,
	                    var_check_lists.fd_list);
	print_scope_tree(var_check_lists.scope_tree);
	std::cout << std::endl;
}
