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
	CHAR_QUOTE,
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
	if (c == ' ' or c == '\t' or c == '\n' or c == '\r') {
		return CHAR_SPACE;
	}
	if (c == 39 || c == 34) {
		return CHAR_QUOTE;
	}
	return CHAR_OPERATOR;
}

std::vector<std::string> keyword_list = {
	"let", "const",
	"if", "elseif", "for", "while", "fn", "then", "do",
	"end",
	"type", "from",
	"number", "string", "bool", "void",
	"return", "break", "continue",
	"true", "false",
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

	char quote_char = ' ';
	bool in_comment = false;

	for (int i = 0; i < (int)code_str.length(); i++) {
		char c = code_str[i];
		char nx = ' ';
		if (i + 1 < (int)code_str.length()) {
			nx = code_str[i + 1];
		}
		CharType char_type = get_char_type(c);

		if (c == '-' && nx == '-') {
			if (token_type != NONE) {
				token_finish();
			}
			in_comment = true;
		}

		if (in_comment) {
			if (c == '\n') {
				line++;
				column = 1;
				token_type = NONE;

				in_comment = false;
			}

			continue;
		}

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
		} else if (token_type == STR) {
			if (quote_char == c) {
				str += c;
				token_finish();
			} else {
				str += c;
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
			} else if (char_type == CHAR_QUOTE) {
				if (quote_char != ' ') {
					quote_char = ' ';
				} else {
					quote_char = c;
					token_type = STR;
					str += c;
				}
			}

			if (char_type != CHAR_SPACE && char_type != CHAR_QUOTE) {
				str += c;
			}
		}

		column++;
		if (c == '\n') {
			line++;
			column = 1;
		}
	}

	if (token_type != NONE) {
		token_finish();
	}
}
