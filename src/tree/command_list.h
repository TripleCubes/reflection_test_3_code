#ifndef TREE_COMMAND_LIST_H
#define TREE_COMMAND_LIST_H

struct Branch;

void to_command_list(Branch &result, const Branch &grouped_token_list,
int start_pos, int end_pos);

#endif
