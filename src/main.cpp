#include <iostream>
#include <string>
#include "file/file.h"
#include "tree/types.h"
#include "tree/token_list.h"
#include "tree/grouped_token_list.h"
#include "tree/command_list.h"
#include "tree_to_lua/tree_to_lua.h"
#include "type_checker/type_checker.h"
#include "err_msg/err_msg.h"

int main() {
	std::string code_str = file_to_str("./test.refl");
	std::cout << code_str << std::endl;
	err_msg_set_code_str(code_str);

	Branch token_list;
	to_token_list(token_list, code_str);
	print_branch(token_list, 0);

	Branch grouped_token_list;
	to_grouped_token_list(grouped_token_list, token_list);
	print_branch(grouped_token_list, 0);

	Branch command_list;
	to_command_list(command_list, grouped_token_list, 0,
		grouped_token_list.branch_list.size() - 1);
	print_branch(command_list, 0);

	type_check(command_list);
	std::string type_err_msg_str = get_type_err_msg_str();
	if ((int)type_err_msg_str.length() != 0) {
		std::cout << type_err_msg_str << std::endl;
		return 0;
	}

	std::string lua_str;
	tree_to_lua(lua_str, command_list);
	std::cout << lua_str << std::endl;
	return 0;
}
