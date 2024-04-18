#include "command_list.h"

#include "types.h"
#include "calc_tree.h"
#include <string>

namespace {
bool funccall_end(const Branch &token, const Branch &nx_token) {
	if (token.str == ")"
	&& (nx_token.type == NAME || nx_token.type == KEYWORD)) {
		return true;
	}
	return false;
}

bool is_right_side_value(const Branch &token) {
	if (token.type == NAME || token.type == STR || token.type == NUM) {
		return true;
	}
	if (token.str == "true" || token.str == "false") {
		return true;
	}
	if (token.type == BRACKET && (token.str == ")"
	|| token.str == "]" || token.str == "}")) {
		return true;
	}
	return false;
}

bool right_side_end(const Branch &token, const Branch &nx_token) {
	if (funccall_end(token, nx_token)) {
		return true;
	}
	
	if (is_right_side_value(token)
	&& (nx_token.type == NAME || nx_token.type == KEYWORD)) {
		return true;
	}

	return false;
}

bool is_inc_code_block_count(const Branch &token) {
	if (token.str == "if" || token.str == "for"
	|| token.str == "while" || token.str == "fn") {
		return true;
	}
	return false;
}

bool code_block_end(const Branch &token, int &code_block_count) {
	if (is_inc_code_block_count(token)) {
		code_block_count++;
	}
	if (token.str == "end") {
		code_block_count--;
	}

	if (code_block_count == 0) {
		return true;
	}
	return false;
}

bool curly_bracket_block_end(const Branch &token, int &curly_count) {
	if (token.str == "{") {
		curly_count++;
	}
	if (token.str == "}") {
		curly_count--;

		if (curly_count == 0) {
			return true;
		}
	}

	return false;
}

bool is_return_type(const Branch &token) {
	if (token.type == NAME) {
		return true;
	}

	if (token.type != KEYWORD) {
		return false;
	}

	if (token.str == "number"
	|| token.str == "string"
	|| token.str == "bool"
	|| token.str == "void") {
		return true;
	}

	return false;
}

//bool is_var_type(const Branch &token) {
//	if (!is_return_type(token)) {
//		return false;
//	}
//
//	if (token.str == "void") {
//		return false;
//	}
//
//	return true;
//}

void new_branch_varnew(Branch &branch,
const Branch &grouped_token_list, int start_pos, int end_pos) {
	Branch var_name_token = grouped_token_list
		                    .branch_list[start_pos + 1];
	branch.branch_list.push_back(var_name_token);
	

	Branch var_type_token = grouped_token_list
	                        .branch_list[start_pos + 3];
	var_type_token.type = VAR_TYPE;
	branch.branch_list.push_back(var_type_token);


	Branch plus_4 = grouped_token_list.branch_list[start_pos + 4];
	if (plus_4.type == BRACKET && plus_4.str == "[") {
		Branch array_sz;
		array_sz.type = ARRAY_INDEX;
		array_sz.line = plus_4.line;
		array_sz.column = plus_4.column;
		branch.branch_list.push_back(array_sz);
	}


	int right_side_start_at = 0;
	for (int i = start_pos; i <= end_pos; i++) {
		Branch v = grouped_token_list.branch_list[i];
		if (v.type == OPERATOR && v.str == "=") {
			right_side_start_at = i + 1;
			break;
		}
	}


	Branch right_side_start = grouped_token_list
	                          .branch_list[right_side_start_at];
	Branch right_side;
	right_side.type = RIGHT_SIDE_TEMP;
	right_side.line = right_side_start.line;
	right_side.column = right_side_start.column;
	for (int i = right_side_start_at; i <= end_pos; i++) {
		Branch v = grouped_token_list.branch_list[i];
		right_side.branch_list.push_back(v);
	}
	branch.branch_list.push_back(right_side);


	Branch bracket_group;
	bracket_group.type = BRACKET_ROUND;
	bracket_group.line = right_side.line;
	bracket_group.column = right_side.column;
	to_calc_tree(bracket_group, right_side);
	branch.branch_list.push_back(bracket_group);
}

void new_branch_assign(Branch &branch,
const Branch &grouped_token_list, int start_pos, int end_pos) {
	Branch var_name_token = grouped_token_list
	                        .branch_list[start_pos];
	branch.branch_list.push_back(var_name_token);


	Branch right_side_start = grouped_token_list
	                          .branch_list[start_pos + 2];
	Branch right_side;
	right_side.type = RIGHT_SIDE_TEMP;
	right_side.line = right_side_start.line;
	right_side.column = right_side_start.column;
	for (int i = start_pos + 2; i <= end_pos; i++) {
		Branch v = grouped_token_list.branch_list[i];
		right_side.branch_list.push_back(v);
	}

	branch.branch_list.push_back(right_side);


	Branch bracket_group;
	bracket_group.type = BRACKET_ROUND;
	bracket_group.line = right_side.line;
	bracket_group.column = right_side.column;
	to_calc_tree(bracket_group, right_side);
	branch.branch_list.push_back(bracket_group);
}

void new_branch_funccall(Branch &branch,
const Branch &grouped_token_list, int start_pos, int end_pos) {
	Branch func_name_token = grouped_token_list
	                         .branch_list[start_pos];
	branch.branch_list.push_back(func_name_token);


	Branch argv_start = grouped_token_list
	                         .branch_list[start_pos + 2];
	Branch argv;
	argv.type = ARGV_TEMP;
	argv.line = argv_start.line;
	argv.column = argv_start.column;
	for (int i = start_pos + 2; i <= end_pos - 1; i++) {
		Branch v = grouped_token_list.branch_list[i];
		argv.branch_list.push_back(v);
	}
	argv.branch_list.push_back(func_name_token);

	branch.branch_list.push_back(argv);


	Branch bracket_group;
	bracket_group.type = BRACKET_FUNCCALL;
	bracket_group.line = argv.line;
	bracket_group.column = argv.column;
	to_calc_tree(bracket_group, argv);
	branch.branch_list.push_back(bracket_group);
}

void new_branch_funcnew(Branch &branch,
const Branch &grouped_token_list, int start_pos, int end_pos) {
	int argv_start = start_pos + 3;
	int argv_end = 0;
	int return_type_start = 0;
	int return_type_end = 0;
	for (int i = argv_start ; i <= end_pos; i++) {
		Branch token = grouped_token_list.branch_list[i];

		if (token.str == ")") {
			argv_end = i - 1;
			return_type_start = i + 2;
			break;
		}
	}
	for (int i = return_type_start; i <= end_pos; i += 2) {
		Branch token = grouped_token_list.branch_list[i];
		Branch nx_token;
		if (i + 1 <= end_pos) {
			nx_token = grouped_token_list.branch_list[i + 1];
		}
		if (is_return_type(token) && nx_token.str != ",") {
			return_type_end = i;
			break;
		}
	}

	Branch func_name_token = grouped_token_list
	                         .branch_list[start_pos + 1];
	branch.branch_list.push_back(func_name_token);


	Branch token_argv_start
	       = grouped_token_list.branch_list[argv_start];
	Branch token_return_type_start
	       = grouped_token_list.branch_list[return_type_start];
	Branch token_code_block_start
	       = grouped_token_list.branch_list[return_type_end + 1];

	Branch funcnew_argv;
	funcnew_argv.type = FUNCNEW_ARGV;
	funcnew_argv.line = token_argv_start.line;
	funcnew_argv.column = token_argv_start.column;
	for (int i = argv_start; i <= argv_end; i++) {
		Branch token = grouped_token_list.branch_list[i];
		funcnew_argv.branch_list.push_back(token);
	}
	branch.branch_list.push_back(funcnew_argv);

	Branch return_types;
	return_types.type = RETURN_TYPES;
	return_types.line = token_return_type_start.line;
	return_types.column = token_return_type_start.column;
	for (int i = return_type_start; i <= return_type_end; i++) {
		Branch token = grouped_token_list.branch_list[i];
		return_types.branch_list.push_back(token);
	}
	branch.branch_list.push_back(return_types);

	Branch code_block;
	code_block.type = CODE_BLOCK;
	code_block.line = token_code_block_start.line;
	code_block.column = token_code_block_start.column;
	to_command_list(code_block, grouped_token_list,
	return_type_end + 1, end_pos - 1);
	branch.branch_list.push_back(code_block);
}

void new_branch_if(Branch &branch,
const Branch &grouped_token_list, int start_pos, int end_pos) {
	int conditions_end = 0;
	int code_block_start = 0;
	for (int i = start_pos + 1; i <= end_pos; i++) {
		Branch token = grouped_token_list.branch_list[i];
		if (token.type == KEYWORD && token.str == "then") {
			conditions_end = i - 1;
			code_block_start = i + 1;
			break;
		}
	}

	Branch token_conditions_start
	       = grouped_token_list.branch_list[start_pos + 1];
	Branch token_code_block_start
	       = grouped_token_list.branch_list[code_block_start];

	Branch conditions;
	conditions.type = CONDITIONS_TEMP;
	conditions.line = token_conditions_start.line;
	conditions.column = token_conditions_start.column;
	for (int i = start_pos + 1; i <= conditions_end; i++) {
		Branch token = grouped_token_list.branch_list[i];
		conditions.branch_list.push_back(token);
	}
	branch.branch_list.push_back(conditions);

	Branch code_block;
	code_block.type = CODE_BLOCK;
	code_block.line = token_code_block_start.line;
	code_block.column = token_code_block_start.column;
	to_command_list(code_block, grouped_token_list,
	code_block_start, end_pos - 1);
	branch.branch_list.push_back(code_block);


	Branch bracket_group;
	bracket_group.type = BRACKET_ROUND;
	bracket_group.line = conditions.line;
	bracket_group.column = conditions.column;
	to_calc_tree(bracket_group, conditions);
	branch.branch_list.push_back(bracket_group);
}

void new_branch_elseif(Branch &branch,
const Branch &grouped_token_list, int start_pos, int end_pos) {
	int conditions_end = 0;
	for (int i = start_pos + 1; i <= end_pos; i++) {
		Branch token = grouped_token_list.branch_list[i];
		if (token.type == KEYWORD && token.str == "then") {
			conditions_end = i - 1;
			break;
		}
	}

	Branch token_conditions_start
	       = grouped_token_list.branch_list[start_pos + 1];

	Branch conditions;
	conditions.type = CONDITIONS_TEMP;
	conditions.line = token_conditions_start.line;
	conditions.column = token_conditions_start.column;
	for (int i = start_pos + 1; i <= conditions_end; i++) {
		Branch token = grouped_token_list.branch_list[i];
		conditions.branch_list.push_back(token);
	}
	branch.branch_list.push_back(conditions);


	Branch bracket_group;
	bracket_group.type = BRACKET_ROUND;
	bracket_group.line = conditions.line;
	bracket_group.column = conditions.column;
	to_calc_tree(bracket_group, conditions);
	branch.branch_list.push_back(bracket_group);
}

void new_branch_for_iter(Branch &branch,
const Branch &grouped_token_list, int start_pos, int end_pos,
int code_block_start) {
	Branch iter = grouped_token_list.branch_list[start_pos + 1];
	branch.branch_list.push_back(iter);

	
	int iter_conditions_start = start_pos + 3;
	int iter_conditions_end = code_block_start - 2;
	Branch token_iter_cond_start
	       = grouped_token_list.branch_list[iter_conditions_start];
	Branch token_code_block_start
	       = grouped_token_list.branch_list[code_block_start];

	Branch iter_conditions;
	iter_conditions.type = FOR_ITER_CONDITIONS_TEMP;
	iter_conditions.line = token_iter_cond_start.line;
	iter_conditions.column = token_iter_cond_start.column;
	for (int i = iter_conditions_start; i <= iter_conditions_end; i++){
		Branch token = grouped_token_list.branch_list[i];
		iter_conditions.branch_list.push_back(token);
	}
	Branch for_token = grouped_token_list.branch_list[start_pos];
	iter_conditions.branch_list.push_back(for_token);
	branch.branch_list.push_back(iter_conditions);


	Branch code_block;
	code_block.type = CODE_BLOCK;
	code_block.line = token_code_block_start.line;
	code_block.column = token_code_block_start.column;
	to_command_list(code_block, grouped_token_list,
	code_block_start, end_pos - 1);
	branch.branch_list.push_back(code_block);


	Branch bracket_group;
	bracket_group.type = BRACKET_FUNCCALL;
	bracket_group.line = iter_conditions.line;
	bracket_group.column = iter_conditions.column;
	to_calc_tree(bracket_group, iter_conditions);
	branch.branch_list.push_back(bracket_group);
}

void new_branch_for_each(Branch &branch,
const Branch &grouped_token_list, int start_pos, int end_pos,
int code_block_start) {
	Branch iter = grouped_token_list.branch_list[start_pos + 1];
	Branch val = grouped_token_list.branch_list[start_pos + 3];
	Branch list = grouped_token_list.branch_list[start_pos + 5];
	branch.branch_list.push_back(iter);
	branch.branch_list.push_back(val);
	branch.branch_list.push_back(list);

	Branch token_code_block_start
	       = grouped_token_list.branch_list[code_block_start];

	Branch code_block;
	code_block.type = CODE_BLOCK;
	code_block.line = token_code_block_start.line;
	code_block.column = token_code_block_start.column;
	to_command_list(code_block, grouped_token_list,
	code_block_start, end_pos - 1);
	branch.branch_list.push_back(code_block);
}

void new_branch_for(Branch &branch,
const Branch &grouped_token_list, int start_pos, int end_pos) {
	int code_block_start = 0;
	for (int i = start_pos; i <= end_pos; i++) {
		Branch token = grouped_token_list.branch_list[i];
		if (token.type == KEYWORD && token.str == "do") {
			code_block_start = i + 1;
		}
	}

	Branch plus_2 = grouped_token_list.branch_list[start_pos + 2];
	if (plus_2.type == OPERATOR && plus_2.str == "=") {
		new_branch_for_iter(branch, grouped_token_list, start_pos,
		end_pos, code_block_start);
	} else {
		branch.type = FOREACH;
		new_branch_for_each(branch, grouped_token_list, start_pos,
		end_pos, code_block_start);
	}
}

void new_branch_while(Branch &branch,
const Branch &grouped_token_list, int start_pos, int end_pos) {
	int conditions_end = 0;
	int code_block_start = 0;
	for (int i = start_pos + 1; i <= end_pos; i++) {
		Branch token = grouped_token_list.branch_list[i];
		if (token.type == KEYWORD && token.str == "do") {
			conditions_end = i - 1;
			code_block_start = i + 1;
			break;
		}
	}

	Branch token_conditions_start
	       = grouped_token_list.branch_list[start_pos + 1];
	Branch token_code_block_start
	       = grouped_token_list.branch_list[code_block_start];

	Branch conditions;
	conditions.type = CONDITIONS_TEMP;
	conditions.line = token_conditions_start.line;
	conditions.column = token_conditions_start.column;
	for (int i = start_pos + 1; i <= conditions_end; i++) {
		Branch token = grouped_token_list.branch_list[i];
		conditions.branch_list.push_back(token);
	}
	branch.branch_list.push_back(conditions);

	Branch code_block;
	code_block.type = CODE_BLOCK;
	code_block.line = token_code_block_start.line;
	code_block.column = token_code_block_start.column;
	to_command_list(code_block, grouped_token_list,
	code_block_start, end_pos - 1);
	branch.branch_list.push_back(code_block);


	Branch bracket_group;
	bracket_group.type = BRACKET_ROUND;
	bracket_group.line = conditions.line;
	bracket_group.column = conditions.column;
	to_calc_tree(bracket_group, conditions);
	branch.branch_list.push_back(bracket_group);
}

void to_member(Branch &member,
const Branch &grouped_token_list, int member_start, int member_end) {
	Branch token_name = grouped_token_list.branch_list[member_start];
	member.branch_list.push_back(token_name);

	Branch token_type = grouped_token_list.branch_list[member_start+2];
	member.branch_list.push_back(token_type);

	Branch plus_3 = grouped_token_list.branch_list[member_start + 3];
	if (plus_3.type == BRACKET && plus_3.str == "[") {
		Branch array_index;
		array_index.type = ARRAY_INDEX;
		array_index.line = plus_3.line;
		array_index.column = plus_3.column;
		member.branch_list.push_back(array_index);
	}

	int right_side_start_at = 0;
	for (int i = member_start; i <= member_end; i++) {
		Branch v = grouped_token_list.branch_list[i];
		if (v.type == OPERATOR && v.str == "=") {
			right_side_start_at = i + 1;
			break;
		}
	}

	Branch right_side_start = grouped_token_list
	                          .branch_list[right_side_start_at];
	Branch right_side;
	right_side.type = RIGHT_SIDE_TEMP;
	right_side.line = right_side_start.line;
	right_side.column = right_side_start.column;
	for (int i = right_side_start_at; i <= member_end; i++) {
		Branch v = grouped_token_list.branch_list[i];
		right_side.branch_list.push_back(v);
	}
	member.branch_list.push_back(right_side);

	Branch bracket_group;
	bracket_group.type = BRACKET_ROUND;
	bracket_group.line = right_side.line;
	bracket_group.column = right_side.column;
	to_calc_tree(bracket_group, right_side);
	member.branch_list.push_back(bracket_group);
}

void to_type_content(Branch &content,
const Branch &grouped_token_list, int content_start, int content_end) {
	int start = content_start;

	auto member_finished = [&start, &content,
	&grouped_token_list](int end) -> void {
		Branch start_token
			   = grouped_token_list.branch_list[start];
		Branch member;
		member.type = TYPE_MEMBER;
		member.line = start_token.line;
		member.column = start_token.column;

		to_member(member, grouped_token_list, start, end);
		content.branch_list.push_back(member);
	};

	for (int i = content_start; i <= content_end; i++) {
		Branch v = grouped_token_list.branch_list[i];
		if (v.type == OPERATOR && v.str == ",") {
			member_finished(i - 1);
			start = i + 1;
		}
	}

	if (start < content_end + 1) {
		member_finished(content_end);
	}
}

void new_branch_type(Branch &branch,
const Branch &grouped_token_list, int start_pos, int end_pos) {
	Branch name_token = grouped_token_list.branch_list[start_pos + 1];
	branch.branch_list.push_back(name_token);

	
	int content_start = start_pos + 3;
	int content_end = end_pos - 1;
	Branch content_start_token
	       = grouped_token_list.branch_list[content_start];
	Branch content;
	content.type = TYPE_MEMBER_LIST;
	content.line = content_start_token.line;
	content.column = content_start_token.column;
	to_type_content(content, grouped_token_list,
		content_start, content_end);
	branch.branch_list.push_back(content);
}
}

void to_command_list(Branch &result, const Branch &grouped_token_list,
int start_pos, int end_pos) {
	BranchType command_type = NONE;
	int command_start_pos = 0;

	auto command_finished = [&grouped_token_list, &command_type,
	&command_start_pos, &result](int command_end_pos) -> void {
		Branch start_token = grouped_token_list
		                     .branch_list[command_start_pos];
			
		Branch new_branch;
		new_branch.type = command_type;
		new_branch.line = start_token.line;
		new_branch.column = start_token.column;
		if (command_type == VARNEW) {
			new_branch_varnew(new_branch, grouped_token_list,
			command_start_pos, command_end_pos);
		} else if (command_type == ASSIGN) {
			new_branch_assign(new_branch, grouped_token_list,
			command_start_pos, command_end_pos);
		} else if (command_type == FUNCCALL) {
			new_branch_funccall(new_branch, grouped_token_list,
			command_start_pos, command_end_pos);
		} else if (command_type == FUNCNEW) {
			new_branch_funcnew(new_branch, grouped_token_list,
			command_start_pos, command_end_pos);
		} else if (command_type == IF) {
			new_branch_if(new_branch, grouped_token_list,
			command_start_pos, command_end_pos);
		} else if (command_type == ELSEIF) {
			new_branch_elseif(new_branch, grouped_token_list,
			command_start_pos, command_end_pos);
		} else if (command_type == ELSE) {
		} else if (command_type == FOR) {
			new_branch_for(new_branch, grouped_token_list,
			command_start_pos, command_end_pos);
		} else if (command_type == WHILE) {
			new_branch_while(new_branch, grouped_token_list,
			command_start_pos, command_end_pos);
		} else if (command_type == TYPE) {
			new_branch_type(new_branch, grouped_token_list,
			command_start_pos, command_end_pos);
		}

		result.branch_list.push_back(new_branch);
		command_type = NONE;
	};

	int code_block_count = 0;
	int curly_count = 0;
	
	for (int i = start_pos; i <= end_pos; i++) {
		Branch token = grouped_token_list.branch_list[i];
		Branch nx_token;
		if (i + 1 < (int)grouped_token_list.branch_list.size()) {
			nx_token = grouped_token_list.branch_list[i + 1];
		}

		if (command_type == NONE) {
			if (token.str == "let") {
				command_start_pos = i;
				command_type = VARNEW;
			}
			else if (token.type == NAME && nx_token.str == "=") {
				command_start_pos = i;
				command_type = ASSIGN;
			}
			else if (token.type == NAME && nx_token.str == "(") {
				command_start_pos = i;
				command_type = FUNCCALL;
			}
			else if (token.str == "fn") {
				command_start_pos = i;
				code_block_count = 0;
				command_type = FUNCNEW;
			}
			else if (token.str == "if") {
				command_start_pos = i;
				code_block_count = 0;
				command_type = IF;
			}
			else if (token.str == "elseif") {
				command_start_pos = i;
				command_type = ELSEIF;
			}
			else if (token.str == "else") {
				command_start_pos = i;
				command_type = ELSE;
			}
			else if (token.str == "for") {
				command_start_pos = i;
				code_block_count = 0;
				command_type = FOR;
			}
			else if (token.str == "while") {
				command_start_pos = i;
				code_block_count = 0;
				command_type = WHILE;
			}
			else if (token.str == "type") {
				command_start_pos = i;
				curly_count = 0;
				command_type = TYPE;
			}
		}
		if (command_type == VARNEW) {
			if (right_side_end(token, nx_token)) {
				command_finished(i);
			}
		}
		if (command_type == ASSIGN) {
			if (right_side_end(token, nx_token)) {
				command_finished(i);
			}
		}
		if (command_type == FUNCCALL) {
			if (funccall_end(token, nx_token)) {
				command_finished(i);
			}
		}
		if (command_type == FUNCNEW) {
			if (code_block_end(token, code_block_count)) {
				command_finished(i);
			}
		}
		if (command_type == IF) {
			if (code_block_end(token, code_block_count)) {
				command_finished(i);
			}
		}
		if (command_type == ELSEIF) {
			if (token.str == "then") {
				command_finished(i);
			}
		}
		if (command_type == ELSE) {
			command_finished(i);
		}
		if (command_type == FOR) {
			if (code_block_end(token, code_block_count)) {
				command_finished(i);
			}
		}
		if (command_type == WHILE) {
			if (code_block_end(token, code_block_count)) {
				command_finished(i);
			}
		}
		if (command_type == TYPE) {
			if (curly_bracket_block_end(token, curly_count)) {
				command_finished(i);
			}
		}
	}

	if (command_type != NONE) {
		command_finished(end_pos);
	}
}
