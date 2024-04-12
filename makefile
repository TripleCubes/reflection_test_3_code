all:
	g++ main.cpp file/file.cpp tree/types.cpp tree/token_list.cpp tree/grouped_token_list.cpp tree/command_list.cpp -o build/main.exe -Wall -g3
