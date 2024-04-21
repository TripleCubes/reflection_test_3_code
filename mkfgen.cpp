#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <fstream>
#include <algorithm>

void check_folder(std::vector<std::string> &cpp_file_list,
const std::string &folder_path, const std::string &src_path) {
	using namespace std::filesystem;

	for (const auto &f: directory_iterator(folder_path)){
		if (f.is_directory()) {
			check_folder(cpp_file_list, f.path().string(), src_path);
		}
		else if (f.path().string().find(".cpp") != std::string::npos) {
			std::string replaced = f.path().string();
			std::replace(replaced.begin(),replaced.end(),(char)92,'/');
			replaced.replace(0, (int)src_path.length(), "");
			cpp_file_list.push_back(replaced);
		}
	}
}

void write_file(const std::string &file_path, const std::string &str) {
	std::ofstream ofs;
	ofs.open(file_path);
	ofs << str;
	ofs.close();
}

int main() {
	std::vector<std::string> cpp_file_list;
	std::string src_path = "./src/";
	std::string build_path = "./build/";
	std::string output_path = "./build/bin/reflection";
	check_folder(cpp_file_list, src_path, src_path);

	for (int i = 0; i < (int)cpp_file_list.size(); i++) {
		std::cout << cpp_file_list[i] << std::endl;
	}

	std::string link_rule = "all: ";
	std::string link_str = "\tg++ -Wall -g3 -o " + output_path + " ";
	std::string depend_str = "-include ";
	std::string obj_str;
	for (int i = 0; i < (int)cpp_file_list.size(); i++) {
		std::string file_path_full = cpp_file_list[i];
		int sz = file_path_full.length();
		std::string file_path = file_path_full.substr(0, sz - 4);
		std::string file_path_underscore = file_path;
		for (int j = (int)file_path_underscore.length()-1; j>=0; j--) {
			char c = file_path_underscore[j];
			if (c == '/') {
				file_path_underscore.insert(j, "_");
			}
		}
		std::replace(file_path_underscore.begin(),
		             file_path_underscore.end(),
		             '/', '_');

		std::string o_path = build_path + "obj/"
		                     + file_path_underscore + ".o";
		std::string d_path = build_path + "obj/"
		                     + file_path_underscore + ".d";
		std::string cpp_path = src_path + file_path + ".cpp";

		link_rule += o_path;
		link_str += o_path;
		depend_str += d_path;
		if (i != (int)cpp_file_list.size() - 1) {
			link_rule += " ";
			link_str += " ";
			depend_str += " ";
		}

		obj_str += o_path + ": " + cpp_path + " makefile\n";

		obj_str += "\tg++ -Wall -g3 -MMD -MP -c " + cpp_path;
		obj_str += " -o " + o_path + "\n";
		
		if (i != (int)cpp_file_list.size() - 1) {
			obj_str += "\n";
		}
	}
	link_rule += "\n";
	link_str += "\n\n";
	depend_str += "\n\n";

	write_file("./makefile", link_rule + link_str
			                 + depend_str + obj_str);
}
