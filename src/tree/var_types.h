#ifndef TREE_VAR_TYPES_H
#define TREE_VAR_TYPES_H

#include <string>

struct Branch;

bool is_return_type(const Branch &token);
bool is_var_type(const Branch &token);
bool is_primitive(const std::string &str);

#endif
