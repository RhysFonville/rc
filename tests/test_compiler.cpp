#include <string>
#include <filesystem>
#include <iostream>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

std::vector<std::string> get_files(const std::string &dir) {
	std::vector<std::string> ret;
    for (const auto &entry : fs::directory_iterator(dir))
		ret.push_back(entry.path());

	return ret;
}

int main(int argc, char* argv[]) {
	std::vector<std::string> files = get_files("tests");
	for (int i = 0; i < files.size(); i++) {
		std::cout << "+======================== test" + std::to_string(i) + " ========================+" << std::endl;
		try {
			system(("../rc tests/test" + std::to_string(i) + "/main.txt -od tests/test"+std::to_string(i)).c_str());
		} catch (const std::exception &e) {
			std::cout << "ERROR: Test #" << std::to_string(i) << " did not work. Message: " << e.what() << std::endl;
		}

		std::cout << "+=======================================================+" << std::endl;
	}
}

