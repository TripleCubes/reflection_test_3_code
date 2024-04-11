#include <iostream>
#include <string>
#include "file/file.h"
#include "tree/types.h"
#include "tree/token_list.h"
#include "tree/grouped_token_list.h"

int main() {
	std::string code_str = file_to_str("./test.refl");
	code_str += ' ';
	std::cout << "INPUT" << std::endl;
	std::cout << code_str << std::endl;
	std::cout << std::endl;

	Branch token_list;
	to_token_list(token_list, code_str);
	print_branch(token_list, 0);

	Branch grouped_token_list;
	to_grouped_token_list(grouped_token_list, token_list);
	print_branch(grouped_token_list, 0);
	return 0;
}
