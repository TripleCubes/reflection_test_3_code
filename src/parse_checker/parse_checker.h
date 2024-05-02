#ifndef PARSE_CHECKER_PARSE_CHECKER_H
#define PARSE_CHECKER_PARSE_CHECKER_H

struct Branch;
enum BranchType: int;

bool parse_check(const Branch &token_list, BranchType branch_type,
int start, int end);

#endif
