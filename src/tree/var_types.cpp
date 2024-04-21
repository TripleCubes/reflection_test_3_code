#include "var_types.h"

#include "types.h"

bool is_return_type(const Branch &token) {
	if (token.type == NAME) {
		return true;
	}

	if (token.type != KEYWORD) {
		return false;
	}

	if (token.str == "number"
	|| token.str == "string"
	|| token.str == "bool"
	|| token.str == "void"
	|| token.str == "fn") {
		return true;
	}

	return false;
}

bool is_var_type(const Branch &token) {
	if (!is_return_type(token)) {
		return false;
	}

	if (token.str == "void") {
		return false;
	}

	return true;
}

bool is_primitive(const std::string &str) {
	if (str == "number"
	|| str == "string"
	|| str == "bool") {
		return true;
	}

	return false;
}
