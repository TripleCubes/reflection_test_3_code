#include "lib_func.h"

#include <vector>

namespace {
std::vector<std::string> skipped_func_list = {
	"print", "void",
	"table.insert", "void",
	"tonumber", "number",
	"tostring", "string",
	"math.random", "number",
	"math.sin", "number",
	"math.cos", "number",
	"math.tan", "number",
	"math.atan", "number",
	"math.sqrt", "number",
};
}

std::string get_lib_func_return_type(const std::string &func_name) {
	for (int i = 0; i < (int)skipped_func_list.size(); i += 2) {
		if (func_name == skipped_func_list[i]) {
			return skipped_func_list[i + 1];
		}
	}

	return "";
}
