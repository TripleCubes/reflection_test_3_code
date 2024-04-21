#include <fstream>
#include <sstream>

std::string file_to_str(const std::string &path) {
	std::ifstream i(path);
	std::stringstream ss;
	ss << i.rdbuf();
	return ss.str();
}
