#ifndef TYPE_CHECKER_VAR_CHECK_H
#define TYPE_CHECKER_VAR_CHECK_H

#include <string>
#include <vector>
#include "../tree/types.h"

struct VarDeclare {
	std::string var_name;
	std::string var_type;
	int scope_id = 0;
	Branch branch;
};

struct TypeDeclare {
	std::string type_name;
	std::string inherit;
	std::vector<VarDeclare> member_list;
	Branch inherit_branch;
};

struct FuncDeclare {
	int var_declare_index;
	std::vector<std::string> argv;
	std::string return_type;
};

std::string get_var_type(
	const std::vector<VarDeclare> &var_declare_list,
	const std::string &var_name,
	const std::vector<int> &scope_tree, int scope_id);

void get_argv_type_list(
	std::vector<std::string> &argv_type_list,
	const std::string &func_name,
	const std::vector<VarDeclare> vd_list,
	const std::vector<FuncDeclare> fd_list,
	const std::vector<int> scope_tree,
	int this_scope);

std::string get_return_type(
	const std::string &func_name,
	const std::vector<VarDeclare> vd_list,
	const std::vector<FuncDeclare> fd_list,
	const std::vector<int> scope_tree,
	int this_scope);

int get_type_declare_i(
	const std::vector<TypeDeclare> &type_declare_list,
	const std::string &type_name);

bool type_compatible(const std::vector<TypeDeclare> &td_list,
	const std::string &type1,
	const std::string &type2);

void add_var_declare(
	std::vector<VarDeclare> &var_declare_list,
	const std::vector<TypeDeclare> &type_declare_list,
	std::vector<FuncDeclare> &func_declare_list,
	VarDeclare declare);

void print_declare_lists(
	const std::vector<VarDeclare> &var_declare_list,
	const std::vector<TypeDeclare> &type_declare_list,
	const std::vector<FuncDeclare> &func_declare_list);

void print_scope_tree(const std::vector<int> &scope_tree);

#endif
