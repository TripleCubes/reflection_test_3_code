#ifndef TYPE_CHECKER_VAR_CHECK_H
#define TYPE_CHECKER_VAR_CHECK_H

#include <string>
#include <vector>

struct VarDeclare {
	std::string var_name;
	std::string var_type;
	int scope_id = 0;
};

struct TypeDeclare {
	std::string type_name;
	std::vector<VarDeclare> member_list;
};

std::string get_var_type(
	const std::vector<VarDeclare> &var_declare_list,
	const std::string &var_name,
	const std::vector<int> &scope_tree, int scope_id);

int get_type_declare_i(
	const std::vector<TypeDeclare> &type_declare_list,
	const std::string &type_name);

void add_var_declare(
	std::vector<VarDeclare> &var_declare_list,
	const std::vector<TypeDeclare> &type_declare_list,
	VarDeclare declare);

void print_declare_lists(
	const std::vector<VarDeclare> &var_declare_list,
	const std::vector<TypeDeclare> &type_declare_list);

void print_scope_tree(const std::vector<int> &scope_tree);

#endif
