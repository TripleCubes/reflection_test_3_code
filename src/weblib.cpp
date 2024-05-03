#include "reflection_test_3.h"
#include <string>
#include <cstring>

namespace {
std::string result_code;
std::string error_str;
}

extern "C" {
int test3_gen(const char *code_str) {
	result_code = "";
	error_str = "";

	std::string code_std_str(code_str);
	bool success = reflection_test_3(code_std_str, result_code,
	                                 error_str);
	if (success) {
		return 1;
	}
	return 0;
}

int get_result_code_sz() {
	return (int)result_code.size();
}

int get_error_str_sz() {
	return (int)error_str.size();
}

void get_result_code(char *output_str) {
	std::strcpy(output_str, result_code.c_str());
}

void get_error_str(char *output_str) {
	std::strcpy(output_str, error_str.c_str());
}
}
