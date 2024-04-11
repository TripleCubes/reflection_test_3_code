#ifndef TREE_TYPES_H
#define TREE_TYPES_H

#include <string>
#include <vector>

enum BranchType {
	NONE        ,

	NUM         ,
	NAME        ,
	STR         ,
	BRACKET     ,
	OPERATOR    ,
	KEYWORD     ,
};

std::string to_str(BranchType branch_type);

struct Branch {
	BranchType type = NONE;
	std::string str;
	int line = -1;
	int column = -1;
	std::vector<Branch> branch_list;
};

void print_branch(Branch branch, int indent);

#endif
