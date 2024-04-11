#include "grouped_token_list.h"

#include "types.h"
#include <vector>

void to_grouped_token_list(Branch &result, const Branch &token_list) {
	bool grouping = false;
	std::vector<Branch> grouped_list;

	auto grouping_finished = [&result, &grouped_list,
	&grouping]() -> void {
		Branch new_branch;
		new_branch.type = NAME;
		new_branch.branch_list = grouped_list;
		new_branch.line = grouped_list[0].line;
		new_branch.column = grouped_list[0].column;
		
		result.branch_list.push_back(new_branch);

		grouping = false;
		grouped_list.clear();
	};

	for (int i = 0; i < (int)token_list.branch_list.size(); i++) {
		Branch token = token_list.branch_list[i];
		Branch nx_token;
		if (i + 1 < (int)token_list.branch_list.size()) {
			nx_token = token_list.branch_list[i + 1];
		}

		if (!grouping) {
			if (token.type == NAME && nx_token.str == ".") {
				grouping = true;
			}
			if (token.type == NAME && nx_token.str == "[") {
				grouping = true;
			}
			else {
				result.branch_list.push_back(token);
			}
		}

		if (grouping) {
			if (token.type == NAME && nx_token.str == ".") {
				grouped_list.push_back(token);
			}
			else if (token.str == "." && nx_token.type == NAME) {
				grouped_list.push_back(token);
			}
			else if ((token.type == NAME || token.type == NUM)
			&& (nx_token.str == "[" || nx_token.str == "]")) {
				grouped_list.push_back(token);
			}
			else if ((token.str == "[" || token.str == "]")
			&& (nx_token.type == NAME || nx_token.type == NUM)) {
				grouped_list.push_back(token);
			}
			else {
				grouped_list.push_back(token);
				grouping_finished();
			}
		}
	}
}
