#ifndef TREE_CALC_TREE_REVERSE_OPERATOR_H
#define TREE_CALC_TREE_REVERSE_OPERATOR_H

#include <string>

struct Branch;
enum BranchType: int;

void reverse_operator(Branch &bracket, const std::string &op);

#endif
