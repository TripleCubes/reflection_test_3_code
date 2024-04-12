#ifndef TREE_CALC_TREE_SHARED_H
#define TREE_CALC_TREE_SHARED_H

struct Branch;
enum BranchType: int;

int get_bracket_size(const Branch &bracket);
bool is_bracket_type(BranchType type);

#endif
