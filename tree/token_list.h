#ifndef TREE_TOKEN_LIST_H
#define TREE_TOKEN_LIST_H

#include <string>

struct Branch;

void to_token_list(Branch &result, const std::string &code_str);

#endif
