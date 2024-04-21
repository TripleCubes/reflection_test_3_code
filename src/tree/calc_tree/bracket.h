#ifndef TREE_CALC_TREE_BRACKET_H
#define TREE_CALC_TREE_BRACKET_H

struct Branch;

void to_bracket_tree(Branch &result, const Branch &token_list,
int start_pos, int end_pos);

#endif
