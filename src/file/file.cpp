#include <fstream>
#include <sstream>
#include <iostream>

bool file_exist(const std::string &path) {
	std::ifstream i(path);
	return i.good();
}

std::string file_to_str(const std::string &path) {
	std::ifstream i(path);
	if (!i.good()) {
		std::cout << path << " not found" << std::endl;
		return "";
	}

	std::stringstream ss;
	ss << i.rdbuf();
	return ss.str();
}

void write_to_file(const std::string &path, const std::string &str) {
	std::ofstream o;
	o.open(path);
	o << str;
	o.close();
}
