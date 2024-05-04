#include "lib_func.h"

#include <vector>

namespace {
std::vector<std::string> skipped_func_list = {
	"print", "number",

	"array_insert", "void",
	"array_size", "number",
	"array_remove", "void",

	"to_number", "number",
	"to_string", "string",
	
	"random_seed", "void",
	"randomf", "number",
	"randomi", "number",
	"sin", "number",
	"cos", "number",
	"tan", "number",
	"atan", "number",
	"sqrt", "number",
	
	"circ", "void",
	"circb", "void",
	"elli", "void",
	"ellib", "void",
	"clip", "void",
	"cls", "void",
	"font", "number",
	"line", "void",
	"map", "void",
	"pix", "number",
	"rect", "void",
	"rectb", "void",
	"spr", "void",
	"tri", "void",
	"trib", "void",
	"ttri", "void",

	"btn", "bool",
	"btnp", "bool",
	"key", "bool",
	"keyp", "bool",
	
	"music", "void",
	"sfx", "void",
	
	"memcpy", "void",
	"memset", "void",
	"pmem", "number",
	"peek", "number",
	"peek1", "number",
	"peek2", "number",
	"peek4", "number",
	"poke", "void",
	"poke1", "void",
	"poke2", "void",
	"poke4", "void",
	"sync", "void",
	"vbank", "void",

	"fget", "bool",
	"fset", "void",
	"mget", "number",
	"mset", "void",
	"void", "void",
	"exit", "void",
	"reset", "void",
	"time", "number",
	"tstamp", "number",
	"trace", "void",
};

std::string lib_func_str =
	"function array_insert(arr, v)\n"
	"	table.insert(arr, v)\n"
	"end\n"
	"\n"
	"function array_size(arr)\n"
	"	return #arr\n"
	"end\n"
	"\n"
	"function array_remove(arr, i)\n"
	"	table.remove(arr, i)\n"
	"end\n"
	"\n"
	"function to_number(str)\n"
	"	return tonumber(str)\n"
	"end\n"
	"\n"
	"function to_string(num)\n"
	"	return tostring(num)\n"
	"end\n"
	"\n"
	"function random_seed()\n"
	"	math.randomseed(os.time())\n"
	"end\n"
	"\n"
	"function randomf(a, b)\n"
	"	return math.random() * (b - a) + a\n"
	"end\n"
	"\n"
	"function randomi(a, b)\n"
	"	return math.random(a, b)\n"
	"end\n"
	"\n"
	"function sin(num)\n"
	"	return math.sin(num)\n"
	"end\n"
	"\n"
	"function cos(num)\n"
	"	return math.cos(num)\n"
	"end\n"
	"\n"
	"function tan(num)\n"
	"	return math.tan(num)\n"
	"end\n"
	"\n"
	"function atan(num)\n"
	"	return math.atan(num)\n"
	"end\n"
	"\n"
	"function sqrt(num)\n"
	"	return math.sqrt(num)\n"
	"end\n"
	"\n";
}

std::string get_lib_func_return_type(const std::string &func_name) {
	for (int i = 0; i < (int)skipped_func_list.size(); i += 2) {
		if (func_name == skipped_func_list[i]) {
			return skipped_func_list[i + 1];
		}
	}

	return "";
}

std::string get_lib_func_str() {
	return lib_func_str;
}
