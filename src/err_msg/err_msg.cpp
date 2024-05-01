#include "err_msg.h"

#include "../tree/types.h"
#include <iostream>
#include <string>

namespace {
std::string code_str;
std::string type_err_msg_str;

std::string err_line(int line) {
	std::string result;
	int current_line = 1;
	for (int i = 0; i < (int)code_str.length(); i++) {
		char c = code_str[i];
		if (c == '\n') {
			current_line++;
		}
		else if (current_line == line) {
			if (c == '\t') {
				result += ' ';
			}
			else {
				result += c;
			}
		}
	}
	return result;
}

std::string arrow(int column) {
	std::string result;
	for (int i = 0; i < column - 2; i++) {
		result += " ";
	}
	result += "^";
	return result;
}
}

void err_msg(const Branch &branch, ErrMsgType type) {
	std::string s;
	switch (type) {
	case EXPECT_VALUE_AT_START:
		s = " value expected";
		break;
	case EXPECT_OP:
		s = " operator expected";
		break;
	case EXPECT_VALUE:
		s = " value expected";
		break;
	case EXPECT_CLOSE_ROUND_BRACKET:
		s = " ')' expected";
		break;
	case EXPECT_CLOSE_SQUARE_BRACKET:
		s = " ']' expected";
		break;
	case EXPECT_CLOSE_CURLY_BRACKET:
		s = " '}' expected";
		break;
	case UNEXPECTED_CLOSE_ROUND_BRACKET:
		s = " ')' not expected";
		break;
	case OPEN_CURLY_IN_ASSIGN:
		s = " struct init only allowed in var initialization";
		break;
	
	case CANT_PARSE:
		s = " cant parse";
		break;
	case EXPECT_VAR_TYPE:
		s = " var type expected";
		break;
	case EXPECT_RETURN_TYPE:
		s = " return type expected";
		break;
	
	case EXPECT_COLON:
		s = " ':' expected";
		break;
	case EXPECT_COMMA:
		s = " ',' expected";
		break;
	case EXPECT_ARROW:
		s = " '->' expected";
		break;
	case EXPECT_VAR_NAME:
		s = " var name expected";
		break;
	case CANT_HAVE_EMPTY_STRUCT:
		s = " struct cant be empty";
		break;

		default:
		s = " unhandled";
		break;
	}

	std::string combined;
	combined += std::to_string(branch.line) + ":" 
		+ std::to_string(branch.column)
		+ s + "\n";
	
	std::string s2;
	std::string s3;
	s2 += "  " + std::to_string(branch.line) + " | ";

	for (int i = 0; i < (int)s2.length() - 2; i++) {
		s3 += " ";
	}

	s2 += err_line(branch.line) + "\n";
	
	s3 += "| ";
	s3 += arrow(branch.column) + "\n";

	combined += s2 + s3 + "\n";

	std::cout << combined << std::endl;

	exit(0);
}

void type_err_msg(const Branch &branch, TypeErrMsgType type,
const std::string &type1, const std::string &type2) {
	std::string s;
	switch (type) {
	case INCOMPATIBLE_OP:
		s = " incompatibe operator";
		break;
	case INCOMPATIBLE_ARGV_NUM:
		s = " incompatible number of arguments";
		break;
	case INCOMPATIBLE_TYPE:
		s = " incompatible type\n";
		s += "expected '" + type1 + "' but got '" + type2 + "'";
		break;
	case TYPE_NOT_DECLARED:
		s = " type not declared";
		break;
	case VAR_NOT_DECLARED:
		s = " var not declared";
		break;
	case FUNC_NOT_DECLARED:
		s = " func not declared";
		break;
	case VAR_ALREADY_DECLARED:
		s = " name already declared";
		break;
	case FUNC_ALREADY_DECLARED:
		s = " name already declared";
		break;
	case TYPE_ALREADY_DECLARED:
		s = " name already declared";
		break;
	case CONTINUE_UNSUPPORTED:
		s = " continue is not supported, yet";
		break;
	case CANT_BREAK:
		s = " can only break inside a loop";
		break;
	case MUST_RETURN_END_OF_FUNC:
		s = " must return at end of function";
		break;
	case INCOMPATIBLE_INHERITANCE_TYPE:
		s = " incompatible inherited type\n";
		s += "expected '" + type1 + "' but got '" + type2 + "'";
		break;
	default:
		s = " type check unhandled";
		break;
	}

	type_err_msg_str += std::to_string(branch.line) + ":" 
		+ std::to_string(branch.column)
		+ s + "\n";
	
	std::string s2;
	std::string s3;
	s2 += "  " + std::to_string(branch.line) + " | ";

	for (int i = 0; i < (int)s2.length() - 2; i++) {
		s3 += " ";
	}

	s2 += err_line(branch.line) + "\n";
	
	s3 += "| ";
	s3 += arrow(branch.column) + "\n";

	type_err_msg_str += s2 + s3 + "\n";
}

void err_msg_set_code_str(const std::string &in_code_str) {
	code_str = in_code_str;
}

std::string get_type_err_msg_str() {
	return type_err_msg_str;
}
