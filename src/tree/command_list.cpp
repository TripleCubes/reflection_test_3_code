#include "command_list.h"

#include "types.h"
#include "var_types.h"
#include "calc_tree.h"
#include "../parse_checker/parse_checker.h"
#include <string>
#include <iostream>

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
		if (code_block_count == 0) {
			return true;
		}
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

void new_branch_varnew(Branch &branch,
const Branch &grouped_token_list, int start_pos, int end_pos) {
	Branch var_name_token = grouped_token_list
		                    .branch_list[start_pos + 1];
	branch.branch_list.push_back(var_name_token);
	

	Branch var_type_token = grouped_token_list
	                        .branch_list[start_pos + 3];
	var_type_token.type = VAR_TYPE;

	Branch plus_4 = grouped_token_list.branch_list[start_pos + 4];
	if (plus_4.type == BRACKET && plus_4.str == "[") {
		Branch array_index;
		array_index.type = ARRAY_INDEX;
		array_index.line = plus_4.line;
		array_index.column = plus_4.column;
		var_type_token.branch_list.push_back(array_index);
	}
	branch.branch_list.push_back(var_type_token);


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

void to_funcnew_argv(Branch &funcnew_argv,
const Branch &funcnew_argv_temp) {
	if ((int)funcnew_argv_temp.branch_list.size() == 0) {
		return;
	}

	int start_pos = 0;

	auto type_finished = [&start_pos,
	&funcnew_argv_temp, &funcnew_argv](int end_pos) -> void {
		Branch start_token = funcnew_argv_temp.branch_list[start_pos];
		Branch argument;
		argument.type = FUNCNEW_ARGUMENT;
		argument.line = start_token.line;
		argument.column = start_token.column;
		argument.branch_list.push_back(start_token);

		Branch plus_2 = funcnew_argv_temp.branch_list[start_pos + 2];
		Branch plus_3;
		if (start_pos + 3 < (int)funcnew_argv_temp.branch_list.size()){
			plus_3 = funcnew_argv_temp.branch_list[start_pos + 3];
		}
		Branch type;
		type.type = VAR_TYPE;
		type.line = plus_2.line;
		type.column = plus_2.column;
		type.str = plus_2.str;
		if (plus_3.type == BRACKET && plus_3.str == "[") {
			Branch array_index;
			array_index.type = ARRAY_INDEX;
			array_index.line = plus_3.line;
			array_index.column = plus_3.column;
			type.branch_list.push_back(array_index);
		}
		argument.branch_list.push_back(type);

		funcnew_argv.branch_list.push_back(argument);
	};

	for (int i = 0; i < (int)funcnew_argv_temp.branch_list.size();i++){
		Branch v = funcnew_argv_temp.branch_list[i];
		if (v.str == ",") {
			type_finished(i - 1);
			start_pos = i + 1;
		}
	}

	type_finished((int)funcnew_argv_temp.branch_list.size() - 1);
}

void to_return_types(Branch &return_types,
const Branch &return_types_temp) {
	int start_pos = 0;

	auto type_finished = [&start_pos,
	&return_types_temp, &return_types](int end_pos) -> void {
		Branch start_token = return_types_temp.branch_list[start_pos];
		Branch type;
		type.type = VAR_TYPE;
		type.line = start_token.line;
		type.column = start_token.column;
		type.str = start_token.str;
		if (end_pos != start_pos) {
			Branch plus_1 = return_types_temp.branch_list[start_pos+1];
			Branch array_index;
			array_index.type = ARRAY_INDEX;
			array_index.line = plus_1.line;
			array_index.column = plus_1.column;
			type.branch_list.push_back(array_index);
		}
		return_types.branch_list.push_back(type);
	};

	for (int i = 0; i < (int)return_types_temp.branch_list.size();i++){
		Branch v = return_types_temp.branch_list[i];
		if (v.str == ",") {
			type_finished(i - 1);
			start_pos = i + 1;
		}
	}

	type_finished((int)return_types_temp.branch_list.size() - 1);
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
		
		bool cond = false;
		if (is_return_type(token) && nx_token.str == "[") {
			cond = true;
		}
		else if (token.str == "[" && nx_token.str == "]") {
			cond = true;
		}
		else if (token.str == "]" && nx_token.str == ",") {
			cond = true;
		}
		else if (token.str == "," && is_return_type(nx_token)) {
			cond = true;
		}
		else if (is_return_type(token) && nx_token.str == ",") {
			cond = true;
		}

		if (!cond) {
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

	Branch funcnew_argv_temp;
	funcnew_argv_temp.type = FUNCNEW_ARGV_TEMP;
	funcnew_argv_temp.line = token_argv_start.line;
	funcnew_argv_temp.column = token_argv_start.column;
	for (int i = argv_start; i <= argv_end; i++) {
		Branch token = grouped_token_list.branch_list[i];
		funcnew_argv_temp.branch_list.push_back(token);
	}
	branch.branch_list.push_back(funcnew_argv_temp);

	Branch return_types_temp;
	return_types_temp.type = RETURN_TYPES_TEMP;
	return_types_temp.line = token_return_type_start.line;
	return_types_temp.column = token_return_type_start.column;
	for (int i = return_type_start; i <= return_type_end; i++) {
		Branch token = grouped_token_list.branch_list[i];
		return_types_temp.branch_list.push_back(token);
	}
	branch.branch_list.push_back(return_types_temp);

	Branch funcnew_argv;
	funcnew_argv.type = FUNCNEW_ARGV;
	funcnew_argv.line = funcnew_argv_temp.line;
	funcnew_argv.column = funcnew_argv_temp.column;
	to_funcnew_argv(funcnew_argv, funcnew_argv_temp);
	branch.branch_list.push_back(funcnew_argv);

	Branch return_types;
	return_types.type = RETURN_TYPES;
	return_types.line = return_types_temp.line;
	return_types.column = return_types_temp.column;
	to_return_types(return_types, return_types_temp);
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

	Branch bracket_group;
	bracket_group.type = BRACKET_ROUND;
	bracket_group.line = conditions.line;
	bracket_group.column = conditions.column;
	to_calc_tree(bracket_group, conditions);
	branch.branch_list.push_back(bracket_group);

	Branch code_block;
	code_block.type = CODE_BLOCK;
	code_block.line = token_code_block_start.line;
	code_block.column = token_code_block_start.column;
	to_command_list(code_block, grouped_token_list,
	code_block_start, end_pos - 1);
	branch.branch_list.push_back(code_block);
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


	Branch bracket_group;
	bracket_group.type = BRACKET_FUNCCALL;
	bracket_group.line = iter_conditions.line;
	bracket_group.column = iter_conditions.column;
	to_calc_tree(bracket_group, iter_conditions);
	branch.branch_list.push_back(bracket_group);


	Branch code_block;
	code_block.type = CODE_BLOCK;
	code_block.line = token_code_block_start.line;
	code_block.column = token_code_block_start.column;
	to_command_list(code_block, grouped_token_list,
	code_block_start, end_pos - 1);
	branch.branch_list.push_back(code_block);
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

	Branch bracket_group;
	bracket_group.type = BRACKET_ROUND;
	bracket_group.line = conditions.line;
	bracket_group.column = conditions.column;
	to_calc_tree(bracket_group, conditions);
	branch.branch_list.push_back(bracket_group);

	Branch code_block;
	code_block.type = CODE_BLOCK;
	code_block.line = token_code_block_start.line;
	code_block.column = token_code_block_start.column;
	to_command_list(code_block, grouped_token_list,
	code_block_start, end_pos - 1);
	branch.branch_list.push_back(code_block);
}

std::string to_lambda_right_side(Branch &lambda_right_side,
const Branch &right_side);

void to_member(Branch &member,
const Branch &grouped_token_list, int member_start, int member_end) {
	Branch token_name = grouped_token_list.branch_list[member_start];
	member.branch_list.push_back(token_name);

	Branch token_type = grouped_token_list.branch_list[member_start+2];
	token_type.type = VAR_TYPE;

	Branch plus_3 = grouped_token_list.branch_list[member_start + 3];
	if (plus_3.type == BRACKET && plus_3.str == "[") {
		Branch array_index;
		array_index.type = ARRAY_INDEX;
		array_index.line = plus_3.line;
		array_index.column = plus_3.column;
		token_type.branch_list.push_back(array_index);
	}
	member.branch_list.push_back(token_type);

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

	if (token_type.str == "fn") {
		Branch var_type_token;
		Branch lambda_right_side;
		lambda_right_side.type = LAMBDA_RIGHT_SIDE;
		lambda_right_side.line = right_side.line;
		lambda_right_side.column = right_side.column;
		var_type_token.str
			= to_lambda_right_side(lambda_right_side, right_side);
		member.branch_list.push_back(lambda_right_side);
		member.branch_list.push_back(var_type_token);
		
		return;
	}

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
		Branch nx;
		if (i + 1 <= content_end) {
			nx = grouped_token_list.branch_list[i + 1];
		}
		Branch nx_nx;
		if (i + 2 <= content_end) {
			nx_nx = grouped_token_list.branch_list[i + 2];
		}

		if (v.type == OPERATOR && v.str == "," && nx_nx.str == ":") {
			member_finished(i - 1);
			start = i + 1;
		}
	}

	Branch end_token = grouped_token_list.branch_list[content_end];
	if (end_token.type == OPERATOR && end_token.str == ",") {
		member_finished(content_end - 1);
	}
	else {
		member_finished(content_end);
	}
}

void new_branch_type(Branch &branch,
const Branch &grouped_token_list, int start_pos, int end_pos) {
	Branch name_token = grouped_token_list.branch_list[start_pos + 1];
	branch.branch_list.push_back(name_token);

	int content_start = start_pos + 3;
	Branch inherit_token;
	Branch from_token = grouped_token_list.branch_list[start_pos + 2];
	if (from_token.type == KEYWORD && from_token.str == "from") {
		content_start = start_pos + 5;
		inherit_token = grouped_token_list.branch_list[start_pos + 3];
	}
	branch.branch_list.push_back(inherit_token);

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

void new_branch_return(Branch &branch,
const Branch &grouped_token_list, int start_pos, int end_pos) {
	if (start_pos == end_pos) {
		return;
	}

	Branch right_side_start
	       = grouped_token_list.branch_list[start_pos + 1];
	Branch right_side;
	right_side.type = RIGHT_SIDE_TEMP;
	right_side.line = right_side_start.line;
	right_side.column = right_side_start.column;
	for (int i = start_pos + 1; i <= end_pos; i++) {
		Branch v = grouped_token_list.branch_list[i];
		right_side.branch_list.push_back(v);
	}

	Branch bracket_group;
	bracket_group.type = BRACKET_ROUND;
	bracket_group.line = right_side.line;
	bracket_group.column = right_side.column;
	to_calc_tree(bracket_group, right_side);
	branch.branch_list.push_back(bracket_group);
}

std::string to_lambda_right_side(Branch &lambda_right_side,
const Branch &right_side) {
	int argv_start = 2;
	int argv_end = 0;
	int return_type_start = 0;
	int return_type_end = 0;
	for (int i = 0 ; i < (int)right_side.branch_list.size(); i++) {
		Branch token = right_side.branch_list[i];

		if (token.str == ")") {
			argv_end = i - 1;
			return_type_start = i + 2;
			break;
		}
	}
	for (int i = return_type_start;
	i < (int)right_side.branch_list.size(); i+=2) {
		Branch token = right_side.branch_list[i];
		Branch nx_token;
		if (i + 1 < (int)right_side.branch_list.size()) {
			nx_token = right_side.branch_list[i + 1];
		}
		
		bool cond = false;
		if (is_return_type(token) && nx_token.str == "[") {
			cond = true;
		}
		else if (token.str == "[" && nx_token.str == "]") {
			cond = true;
		}
		else if (token.str == "]" && nx_token.str == ",") {
			cond = true;
		}
		else if (token.str == "," && is_return_type(nx_token)) {
			cond = true;
		}
		else if (is_return_type(token) && nx_token.str == ",") {
			cond = true;
		}

		if (!cond) {
			return_type_end = i;
			break;
		}
	}


	Branch token_argv_start
	       = right_side.branch_list[argv_start];
	Branch token_return_type_start
	       = right_side.branch_list[return_type_start];
	Branch token_code_block_start
	       = right_side.branch_list[return_type_end + 1];

	Branch funcnew_argv_temp;
	funcnew_argv_temp.type = FUNCNEW_ARGV_TEMP;
	funcnew_argv_temp.line = token_argv_start.line;
	funcnew_argv_temp.column = token_argv_start.column;
	for (int i = argv_start; i <= argv_end; i++) {
		Branch token = right_side.branch_list[i];
		funcnew_argv_temp.branch_list.push_back(token);
	}
	lambda_right_side.branch_list.push_back(funcnew_argv_temp);

	Branch return_types_temp;
	return_types_temp.type = RETURN_TYPES_TEMP;
	return_types_temp.line = token_return_type_start.line;
	return_types_temp.column = token_return_type_start.column;
	for (int i = return_type_start; i <= return_type_end; i++) {
		Branch token = right_side.branch_list[i];
		return_types_temp.branch_list.push_back(token);
	}
	lambda_right_side.branch_list.push_back(return_types_temp);

	Branch funcnew_argv;
	funcnew_argv.type = FUNCNEW_ARGV;
	funcnew_argv.line = funcnew_argv_temp.line;
	funcnew_argv.column = funcnew_argv_temp.column;
	to_funcnew_argv(funcnew_argv, funcnew_argv_temp);
	lambda_right_side.branch_list.push_back(funcnew_argv);

	Branch return_types;
	return_types.type = RETURN_TYPES;
	return_types.line = return_types_temp.line;
	return_types.column = return_types_temp.column;
	to_return_types(return_types, return_types_temp);
	lambda_right_side.branch_list.push_back(return_types);

	Branch code_block;
	code_block.type = CODE_BLOCK;
	code_block.line = token_code_block_start.line;
	code_block.column = token_code_block_start.column;
	to_command_list(code_block, right_side,
	return_type_end + 1, (int)right_side.branch_list.size()-2);
	lambda_right_side.branch_list.push_back(code_block);

	std::string type = "fn (";
	for (int i = 0; i < (int)funcnew_argv_temp.branch_list.size();i++){
		Branch v = funcnew_argv_temp.branch_list[i];
		Branch nx;
		if (i + 1 < (int)funcnew_argv_temp.branch_list.size()) {
			nx = funcnew_argv_temp.branch_list[i + 1];
		}

		if (nx.type == OPERATOR && nx.str == ":") {
			continue;
		}
		if (v.type == OPERATOR && v.str == ":") {
			continue;
		}

		type += v.str;
	}
	type += ") -> ";
	for (int i = 0; i < (int)return_types_temp.branch_list.size();i++){
		Branch v = return_types_temp.branch_list[i];
		Branch nx;
		if (i + 1 < (int)return_types_temp.branch_list.size()) {
			nx = return_types_temp.branch_list[i + 1];
		}

		type += v.str;
	}
	return type;
}

void new_branch_lambda_new(Branch &branch,
const Branch &grouped_token_list, int start_pos, int end_pos) {
	Branch var_name_token = grouped_token_list
		                    .branch_list[start_pos + 1];
	branch.branch_list.push_back(var_name_token);
	

	Branch var_type_token;
	var_type_token.type = VAR_TYPE;


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


	Branch lambda_right_side;
	lambda_right_side.type = LAMBDA_RIGHT_SIDE;
	lambda_right_side.line = right_side.line;
	lambda_right_side.column = right_side.column;
	var_type_token.str
		= to_lambda_right_side(lambda_right_side, right_side);
	branch.branch_list.push_back(lambda_right_side);
	branch.branch_list.push_back(var_type_token);
}

void new_branch_lambda_assign(Branch &branch,
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


	Branch var_type_token;
	var_type_token.type = VAR_TYPE;

	Branch lambda_right_side;
	lambda_right_side.type = LAMBDA_RIGHT_SIDE;
	lambda_right_side.line = right_side.line;
	lambda_right_side.column = right_side.column;
	var_type_token.str
		= to_lambda_right_side(lambda_right_side, right_side);
	branch.branch_list.push_back(lambda_right_side);
	branch.branch_list.push_back(var_type_token);
}
}

void to_command_list(Branch &result, const Branch &grouped_token_list,
int start_pos, int end_pos) {
	BranchType command_type = NONE;
	int command_start_pos = 0;

	auto command_finished = [&grouped_token_list, &command_type,
	&command_start_pos, &result](int command_end_pos) -> void {
		parse_check(grouped_token_list, command_type,
			command_start_pos, command_end_pos);

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
		} else if (command_type == RETURN) {
			new_branch_return(new_branch, grouped_token_list,
			command_start_pos, command_end_pos);
		} else if (command_type == BREAK) {
		} else if (command_type == CONTINUE) {
		} else if (command_type == LAMBDA_NEW) {
			new_branch_lambda_new(new_branch, grouped_token_list,
			command_start_pos, command_end_pos);
		} else if (command_type == LAMBDA_ASSIGN) {
			new_branch_lambda_assign(new_branch, grouped_token_list,
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
		Branch prev_token;
		if (i - 1 >= 0) {
			prev_token = grouped_token_list.branch_list[i - 1];
		}
		Branch nx_3;
		if (i + 3 < (int)grouped_token_list.branch_list.size()) {
			nx_3 = grouped_token_list.branch_list[i + 3];
		}
		Branch nx_2;
		if (i + 2 < (int)grouped_token_list.branch_list.size()) {
			nx_2 = grouped_token_list.branch_list[i + 2];
		}

		if (command_type == NONE) {
			command_start_pos = i;
			if (token.str == "let") {
				if (nx_3.str == "fn") {
					command_type = LAMBDA_NEW;
					code_block_count = -1;
				}
				else {
					command_type = VARNEW;
				}
			}
			else if (token.type == NAME && nx_token.str == "=") {
				command_start_pos = i;
				if (nx_2.str == "fn") {
					command_type = LAMBDA_ASSIGN;
					code_block_count = 0;
				}
				else {
					command_type = ASSIGN;
				}
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
			else if (token.str == "return") {
				command_start_pos = i;
				command_type = RETURN;
			}
			else if (token.str == "break") {
				command_start_pos = i;
				command_type = BREAK;
			}
			else if (token.str == "continue") {
				command_start_pos = i;
				command_type = CONTINUE;
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
		if (command_type == RETURN) {
			if (right_side_end(token, nx_token)) {
				command_finished(i);
			}
			else if (prev_token.str == "return"
			&& (token.type == NAME || token.type == KEYWORD)
			&& nx_token.str != "(") {
				command_finished(i);
			}
		}
		if (command_type == BREAK) {
			command_finished(i);
		}
		if (command_type == CONTINUE) {
			command_finished(i);
		}
		if (command_type == LAMBDA_NEW) {
			if (code_block_end(token, code_block_count)) {
				command_finished(i);
			}
		}
		if (command_type == LAMBDA_ASSIGN) {
			if (code_block_end(token, code_block_count)) {
				command_finished(i);
			}
		}
	}

	if (command_type != NONE) {
		command_finished(end_pos);
	}
}
