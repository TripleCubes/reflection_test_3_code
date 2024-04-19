all:
	g++ main.cpp file/file.cpp tree/types.cpp tree/token_list.cpp tree/grouped_token_list.cpp tree/command_list.cpp tree/calc_tree.cpp tree/calc_tree/bracket.cpp tree/calc_tree/reverse_operator.cpp tree/calc_tree/shared.cpp tree/calc_tree/func_argv.cpp tree/calc_tree/group_by_operator.cpp tree/calc_tree/calc.cpp tree_to_lua/tree_to_lua.cpp -o build/main.exe -Wall -g3
