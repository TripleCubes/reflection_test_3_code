#include "reflection_test_3.h"
#include <string>
#include <iostream>

int main(int argc, char *argv[]) {
	if (argc > 3) {
		std::cout << "too many arguments" << std::endl;
		return 0;
	}
	if (argc == 1) {
		std::cout << "path to code file needed" << std::endl;
		return 0;
	}

	std::string output_path;
	if (argc == 3) {
		std::string argv_str(argv[2]);
		output_path = argv_str;
	}

	std::string input_path(argv[1]);


	std::string result_code;
	std::string error_str;
	bool success = reflection_test_3(
		input_path,
		result_code,
		error_str);

	if (success) {
		std::cout << result_code << std::endl;
	}
	else {
		std::cout << error_str << std::endl;
	}

	return 0;
}
