#include <string>
#include <filesystem>
#include "../compiler.cpp"

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
		clog::out("+======================== TEST #" + std::to_string(i) + " ========================+");
		
		try {
			system((".././rc " + files[i]).c_str());
		} catch (const std::exception &e) {
			clog::error("Test #" + std::to_string(i) + " did not work. Message: " + e.what());
		}

		clog::out("+=========================================================+");
	}
}

