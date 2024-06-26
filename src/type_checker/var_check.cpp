#include "var_check.h"

#include <iostream>
#include "../err_msg/err_msg.h"
#include "../tree/types.h"

namespace {
bool is_in_scope(const std::vector<int> &scope_tree,
int scope_id, int in_scope_id) {
	int checking = scope_id;
	while (checking != -1) {
		if (checking == in_scope_id) {
			return true;
		}

		checking = scope_tree[checking];
	}
	return false;
}

std::string reformat_name(const std::string &var_name) {
	std::string result;

	bool in_bracket = false;

	for (int i = 0; i < (int)var_name.length(); i++) {
		char c = var_name[i];

		if (c == ']') {
			in_bracket = false;
			result += '0';
		}

		if (!in_bracket) {
			result += c;
		}

		if (c == '[' && i + 1 < (int)var_name.length()) {
			in_bracket = true;
		}
	}

	return result;
}

void get_type_declare_member_list(
std::vector<VarDeclare> &result,
const std::vector<TypeDeclare> &td_list, int index) {
	auto is_in_list_and_more = [&result](
	const VarDeclare var_declare) -> bool {
		for (int i = 0; i < (int)result.size(); i++) {
			const VarDeclare &v = result[i];
			if (v.var_name == var_declare.var_name) {
				if (v.var_type != var_declare.var_type) {
					type_err_msg(v.branch,
					             INCOMPATIBLE_INHERITANCE_TYPE,
					             var_declare.var_type, v.var_type);
				}
				return true;
			}
		}
		return false;
	};

	const TypeDeclare &type_declare = td_list[index];
	for (int i = 0; i < (int)type_declare.member_list.size(); i++) {
		const VarDeclare &member = type_declare.member_list[i];
		if (is_in_list_and_more(member)) {
			continue;
		}

		result.push_back(member);
	}

	if (type_declare.inherit == "") {
		return;
	}
	
	for (int i = 0; i < (int)td_list.size(); i++) {
		const TypeDeclare &v = td_list[i];
		if (type_declare.inherit == v.type_name) {
			get_type_declare_member_list(
				result, td_list, i);
			return;
		}
	}

	type_err_msg(type_declare.inherit_branch, TYPE_NOT_DECLARED,"","");
}

void str_argv_from_func_type(std::vector<std::string> &result,
const std::string func_type) {
	int argv_start = 0;
	int argv_end = 0;
	
	for (int i = 0; i < (int)func_type.length(); i++) {
		char c = func_type[i];
		if (c == '(') {
			argv_start = i + 1;
		}
		else if (c == ')') {
			argv_end = i - 1;
		}
	}

	if (argv_end < argv_start) {
		return;
	}

	std::string word;
	for (int i = argv_start; i <= argv_end; i++) {
		char c = func_type[i];
		if (c == ',') {
			result.push_back(word);
			word = "";
		}
		else {
			word += c;
		}
	}

	if (word.length() != 0) {
		result.push_back(word);
	}
}

void str_return_type_from_func_type(std::string &result,
const std::string func_type){
	int return_type_start = 0;

	for (int i = 0; i < (int)func_type.length(); i++) {
		char v = func_type[i];
		char nx = ' ';
		if (i + 1 < (int)func_type.length()) {
			nx = func_type[i + 1];
		}

		if (v == '-' && nx == '>') {
			return_type_start = i + 3;
		}
	}

	result = func_type.substr(return_type_start,
		(int)func_type.length() - return_type_start);
}
}

std::string get_var_type(
const std::vector<VarDeclare> &var_declare_list,
const std::string &var_name,
const std::vector<int> &scope_tree, int scope_id) {
	for (int i = 0; i < (int)var_declare_list.size(); i++) {
		const VarDeclare &v = var_declare_list[i];

		bool cond = is_in_scope(scope_tree, scope_id, v.scope_id);

		if (v.var_name == reformat_name(var_name) && cond) {
			return var_declare_list[i].var_type;
		}
	}
	return "";
}

void get_argv_type_list(
std::vector<std::string> &argv_type_list,
const std::string &func_name,
const std::vector<VarDeclare> vd_list,
const std::vector<FuncDeclare> fd_list,
const std::vector<int> scope_tree,
int this_scope) {
	for (int i = 0; i < (int)fd_list.size(); i++) {
		const FuncDeclare func_declare = fd_list[i];
		int index = func_declare.var_declare_index;
		const VarDeclare &var_declare = vd_list[index];
		if (var_declare.var_name == func_name
		&& is_in_scope(scope_tree, this_scope, var_declare.scope_id)) {
			for (int j = 0; j < (int)func_declare.argv.size(); j++) {
				const std::string &arg_type
					= func_declare.argv[j];
				argv_type_list.push_back(arg_type);
			}
		}
	}
}

std::string get_return_type(
const std::string &func_name,
const std::vector<VarDeclare> vd_list,
const std::vector<FuncDeclare> fd_list,
const std::vector<int> scope_tree,
int this_scope) {
	for (int i = 0; i < (int)fd_list.size(); i++) {
		const FuncDeclare func_declare = fd_list[i];
		int index = func_declare.var_declare_index;
		const VarDeclare &var_declare = vd_list[index];
		if (var_declare.var_name == func_name
		&& is_in_scope(scope_tree, this_scope, var_declare.scope_id)) {
			return func_declare.return_type;
		}
	}
	return "";
}

int get_type_declare_i(
const std::vector<TypeDeclare> &type_declare_list,
const std::string &type_name) {
	for (int i = 0; i < (int)type_declare_list.size(); i++) {
		if (type_declare_list[i].type_name == type_name) {
			return i;
		}
	}
	return -1;
}

bool type_compatible(const std::vector<TypeDeclare> &td_list,
const std::string &type1,
const std::string &type2) {
	if (type1 == type2) {
		return true;
	}
	int index = get_type_declare_i(td_list, type2);
	if (index == -1) {
		return false;
	}
	while (true) {
		std::string inherit = td_list[index].inherit;
		if (inherit == type1) {
			return true;
		}
		if (inherit == "") {
			return false;
		}
		index = get_type_declare_i(td_list, inherit);
	}
	return false;
}

void add_var_declare(
std::vector<VarDeclare> &var_declare_list,
const std::vector<TypeDeclare> &type_declare_list,
std::vector<FuncDeclare> &func_declare_list,
VarDeclare declare) {
	var_declare_list.push_back(declare);

	int var_type_sz = (int)declare.var_type.size();
	if (declare.var_type[var_type_sz - 1] == ']') {
		VarDeclare new_var_declare;
		new_var_declare.var_name = declare.var_name + "[0]";
		new_var_declare.var_type
			= declare.var_type.substr(0, var_type_sz - 2);
		new_var_declare.scope_id = declare.scope_id;
		add_var_declare(var_declare_list,
		                type_declare_list,
		                func_declare_list,
		                new_var_declare);
	}

	if (declare.var_type[0] == 'f' && declare.var_type[1] == 'n') {
		FuncDeclare new_func_declare;
		new_func_declare.var_declare_index
		                 = (int)var_declare_list.size() - 1;
		str_argv_from_func_type(new_func_declare.argv,
		                        declare.var_type);


		VarDeclare new_var_declare;
		new_var_declare.var_name = declare.var_name + "()";
		str_return_type_from_func_type(new_var_declare.var_type,
		                               declare.var_type);
		new_var_declare.scope_id = declare.scope_id;
//		add_var_declare(var_declare_list,
//		                type_declare_list,
//		                func_declare_list,
//		                new_var_declare);


		new_func_declare.return_type = new_var_declare.var_type;
		func_declare_list.push_back(new_func_declare);
	}


	int index = get_type_declare_i(type_declare_list,declare.var_type);
	if (index == -1) {
		return;
	}

	std::vector<VarDeclare> m_list;
	get_type_declare_member_list(m_list, type_declare_list, index);

	for (int i = 0; i < (int)m_list.size(); i++) {
		const VarDeclare &member
		                 = m_list[i];
		VarDeclare new_var_declare;
		new_var_declare.var_name = declare.var_name
		                           + "." + member.var_name;
		new_var_declare.var_type = member.var_type;
		new_var_declare.scope_id = declare.scope_id;
		add_var_declare(var_declare_list,
		                type_declare_list,
		                func_declare_list,
		                new_var_declare);
	}
}

void print_declare_lists(
const std::vector<VarDeclare> &var_declare_list,
const std::vector<TypeDeclare> &type_declare_list,
const std::vector<FuncDeclare> &func_declare_list) {
	for (int i = 0; i < (int)type_declare_list.size(); i++) {
		const TypeDeclare &v = type_declare_list[i];
		std::cout << i << "| " << v.type_name 
			<< " " << v.inherit << std::endl;
		for (int j = 0; j < (int)v.member_list.size(); j++) {
			VarDeclare vv = v.member_list[j];
			std::cout << "    "
			          << vv.var_name
			          << " " << vv.var_type << std::endl;
		}
	}

	std::cout << std::endl;

	for (int i = 0; i < (int)var_declare_list.size(); i++) {
		const VarDeclare &v = var_declare_list[i];
		std::cout << i << "| " << v.var_name << " " << v.var_type
			<< " " << v.scope_id << std::endl;
	}

	std::cout << std::endl;

	for (int i = 0; i < (int)func_declare_list.size(); i++) {
		const FuncDeclare &v = func_declare_list[i];
		std::cout << i << "| " << v.var_declare_index
			<< " " << v.return_type << std::endl;
		std::cout << "    ";
		for (int j = 0; j < (int)v.argv.size(); j++) {
			std::cout << v.argv[j] << " ";
		}
		std::cout << std::endl;
	}

	std::cout << std::endl << std::endl;
}

void print_scope_tree(const std::vector<int> &scope_tree) {
	for (int i = 0; i < (int)scope_tree.size(); i++) {
		std::cout << i << "| " << scope_tree[i] << " <- " << i;
		std::cout << std::endl;
	}
}
