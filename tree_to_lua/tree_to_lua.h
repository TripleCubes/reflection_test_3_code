#ifndef TREE_TO_LUA_TREE_TO_LUA_H
#define TREE_TO_LUA_TREE_TO_LUA_H

#include <string>

struct Branch;

void tree_to_lua(std::string &result, const Branch &tree);

#endif
