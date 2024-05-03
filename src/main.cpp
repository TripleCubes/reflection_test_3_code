#include "reflection_test_3.h"
#include <string>
#include <iostream>
#include "file/file.h"
#include "top_bottom_comments/top_bottom_comments.h"

void write_output_file(const std::string &output_path,
const std::string &result_code) {
	std::string top_comments;
	std::string bottom_comments;

	if (file_exist(output_path)) {
		std::string output_old_str
			= file_to_str(output_path);

		top_comments = get_top_comments(output_old_str);
		bottom_comments = get_bottom_comments(output_old_str);
	}

	std::cout << "written to " << output_path << std::endl;
	write_to_file(output_path,
		top_comments + result_code + bottom_comments);
}

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


	std::string code_str = file_to_str(input_path);
	if (code_str == "") {
		std::cout << "input file is empty or not found" << std::endl;
		return 0;
	}


	std::string result_code;
	std::string error_str;
	bool success = reflection_test_3(
		code_str,
		result_code,
		error_str);


	if (success) {
		if (output_path != "") {
			write_output_file(output_path, result_code);
		}
		else {
			std::cout << result_code << std::endl;
		}
	}
	else {
		std::cout << error_str << std::endl;
	}

	return 0;
}
