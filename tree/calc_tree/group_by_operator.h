#ifndef TREE_CALC_TREE_GROUP_BY_OPERATOR_H
#define TREE_CALC_TREE_GROUP_BY_OPERATOR_H

#include <string>
#include <vector>

struct Branch;

void group_by_operator(Branch &bracket,
const std::vector<std::string> &op_str_list);

#endif
