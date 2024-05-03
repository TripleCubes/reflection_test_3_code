#include "top_bottom_comments.h"

std::string get_top_comments(const std::string &code_str) {
	int comment_end = 0;

	for (int i = 0; i < (int)code_str.size(); i++) {
		char c = code_str[i];
		char nx = ' ';
		if (i + 1 < (int)code_str.size()) {
			nx = code_str[i + 1];
		}
		char prev = ' ';
		if (i - 1 >= 0) {
			prev = code_str[i - 1];
		}

		if (prev == '\n' && c == '\n') {
			continue;
		}

		if (prev == '\n' && (c != '-' || nx != '-') && i - 1 >= 0) {
			comment_end = i - 1;
			break;
		}
	}

	return code_str.substr(0, comment_end) + "\n";
}

std::string get_bottom_comments(const std::string &code_str) {
	int last_comment_pos = 0;

	for (int i = (int)code_str.size() - 1; i >= 0; i--) {
		char c = code_str[i];
		char nx = ' ';
		if (i + 1 < (int)code_str.size()) {
			nx = code_str[i + 1];
		}
		char prev = ' ';
		if (i - 1 >= 0) {
			prev = code_str[i - 1];
		}

		if (prev == '\n' && c == '\n') {
			continue;
		}

		if (prev == '\n' && c == '-' && nx == '-') {
			last_comment_pos = i;
		}

		if (prev == '\n' && (c != '-' || nx != '-')) {
			break;
		}
	}

	return code_str.substr(last_comment_pos,
	                       (int)code_str.size() - last_comment_pos);
}
