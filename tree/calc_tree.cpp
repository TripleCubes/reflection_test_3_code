#include "calc_tree.h"

#include "types.h"
#include "calc_tree/bracket.h"

void to_calc_tree(Branch &result, const Branch &token_list) {
	to_bracket_tree(result, token_list, 0,
		token_list.branch_list.size() - 1);
}
