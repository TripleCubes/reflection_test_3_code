#include "var_check.h"

#include <iostream>

std::string get_var_type(
const std::vector<VarDeclare> &var_declare_list,
const std::string &var_name) {
	for (int i = 0; i < (int)var_declare_list.size(); i++) {
		if (var_declare_list[i].var_name == var_name) {
			return var_declare_list[i].var_type;
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

void add_var_declare(
std::vector<VarDeclare> &var_declare_list,
const std::vector<TypeDeclare> &type_declare_list,
VarDeclare declare) {
	var_declare_list.push_back(declare);

	int index = get_type_declare_i(type_declare_list,declare.var_type);
	if (index == -1) {
		return;
	}

	const TypeDeclare &type_declare
	                  = type_declare_list[index];
	for (int i = 0; i < (int)type_declare.member_list.size(); i++) {
		const VarDeclare &member
		                 = type_declare.member_list[i];
		VarDeclare new_var_declare;
		new_var_declare.var_name = declare.var_name
		                           + "." + member.var_name;
		new_var_declare.var_type = member.var_type;
		add_var_declare(var_declare_list,
		                type_declare_list,
		                new_var_declare);
	}
}

void print_declare_lists(
const std::vector<VarDeclare> &var_declare_list,
const std::vector<TypeDeclare> &type_declare_list) {
	std::cout << "----" << std::endl;
	for (int i = 0; i < (int)type_declare_list.size(); i++) {
		TypeDeclare v = type_declare_list[i];
		std::cout << v.type_name << std::endl;
		for (int j = 0; j < (int)v.member_list.size(); j++) {
			VarDeclare vv = v.member_list[j];
			std::cout << "    "
			          << vv.var_name
			          << " " << vv.var_type << std::endl;
		}
	}

	std::cout << std::endl;

	for (int i = 0; i < (int)var_declare_list.size(); i++) {
		VarDeclare v = var_declare_list[i];
		std::cout << v.var_name << " " << v.var_type << std::endl;
	}
	std::cout << "----" << std::endl;
}
