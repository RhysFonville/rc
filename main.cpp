#include "compiler.cpp"

int main(int argc, char* argv[]) {
	std::vector<std::string> args;
	for (int i = 0; i < argc; i++) {
		args.push_back(std::string(argv[i]));
	}

	std::ifstream read;
	read.open(args[1], std::ifstream::in);
	std::string l;	
	while (std::getline(read, l)) {
		lines.push_back(l);
	}	
	read.close();

	begin_preprocessing();
	begin_compile(args);

	return 0;
}
