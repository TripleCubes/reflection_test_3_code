#include "token_list.h"

#include <vector>
#include "types.h"

namespace {
enum CharType {
	CHAR_NONE,
	CHAR_NUM,
	CHAR_TXT,
	CHAR_SPACE,
	CHAR_BRACKET,
	CHAR_OPERATOR,
};

CharType get_char_type(char c) {
	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
	|| c == '_') {
		return CHAR_TXT;
	}
	if (c >= '0' && c <= '9') {
		return CHAR_NUM;
	}
	if (c == '(' || c == ')' || c == '[' || c == ']'
	|| c == '{' || c == '}') {
		return CHAR_BRACKET;
	}
	if (c == ' ' or c == '\t' or c == '\n') {
		return CHAR_SPACE;
	}
	return CHAR_OPERATOR;
}

std::vector<std::string> keyword_list = {
	"let", "const",
	"if", "elseif", "for", "while", "fn",
	"end",
	"type",
};

bool is_keyword(const std::string &str) {
	for (int i = 0; i < (int)keyword_list.size(); i++) {
		if (keyword_list[i] == str) {
			return true;
		}
	}
	return false;
}

bool is_operator(const std::string &str) {
	if (str == "not" || str == "and" || str == "or") {
		return true;
	}
	return false;
}
}

void to_token_list(Branch &result, const std::string &code_str) {
	BranchType token_type = NONE;
	std::string str;
	int line = 1;
	int column = 1;

	auto token_finish = [&result, &token_type,
	&str, &line, &column]() -> void {
		Branch branch;
		branch.type = token_type;
		branch.str = str;
		branch.line = line;
		branch.column = column;

		if (is_keyword(str)) {
			branch.type = KEYWORD;
		} else if (is_operator(str)) {
			branch.type = OPERATOR;
		}

		result.branch_list.push_back(branch);

		token_type = NONE;
		str = "";
	};

	for (int i = 0; i < (int)code_str.length(); i++) {
		char c = code_str[i];
		CharType char_type = get_char_type(c);

		if (token_type == NAME) {
			if (char_type == CHAR_TXT || char_type == CHAR_NUM) {
				str += c;
			} else {
				token_finish();
			}
		} else if (token_type == NUM) {
			if (char_type == CHAR_NUM || c == '.') {
				str += c;
			} else {
				token_finish();
			}
		} else if (token_type == BRACKET) {
			token_finish();
		} else if (token_type == OPERATOR) {
			if (char_type == CHAR_OPERATOR) {
				str += c;
			} else {
				token_finish();
			}
		}

		if (token_type == NONE) {
			if        (char_type == CHAR_TXT) {
				token_type = NAME;
			} else if (char_type == CHAR_NUM) {
				token_type = NUM;
			} else if (char_type == CHAR_BRACKET) {
				token_type = BRACKET;
			} else if (char_type == CHAR_OPERATOR) {
				token_type = OPERATOR;
			}

			if (char_type != CHAR_SPACE) {
				str += c;
			}
		}

		column++;
		if (c == '\n') {
			line++;
			column = 1;
		}
	}
}
