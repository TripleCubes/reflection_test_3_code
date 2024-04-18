#include "calc_tree.h"

#include "types.h"
#include "calc_tree/bracket.h"
#include "calc_tree/reverse_operator.h"
#include "calc_tree/func_argv.h"
#include "calc_tree/group_by_operator.h"
#include "calc_tree/calc.h"

void to_calc_tree(Branch &result, const Branch &token_list) {
	to_bracket_tree(result, token_list, 0,
		token_list.branch_list.size() - 1);
	reverse_operator(result, "-", REVERSE_SIGN);
	reverse_operator(result, "not", REVERSE_COND);
	func_argv(result);
	group_by_operator(result, { "*", "/", "//", "%" });
	group_by_operator(result, { "-", "+" });
	group_by_operator(result, { "==", "~=", ">=", "<=", ">", "<" });
	bracket_to_calc(result);
}
