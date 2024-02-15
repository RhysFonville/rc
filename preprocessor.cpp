#include <fstream>
#include "util.h"

namespace token_function {
	void macro(TokIt tok_it) {
		if (*(tok_it+1) == "inc") {
			if (*(tok_it+2) != "\"" || *(tok_it+4) != "\"") {
				clog::error("Include macro parameter must be wrapped in double quotes in a valid manner.");
			}
			
			std::ifstream in(*(tok_it+3));
			
			std::vector<std::string> inc_lines = { };
			std::string l;
			while (std::getline(in, l)) {
				inc_lines.push_back(l);
			}
			
			lines.insert(lines.begin()+line_number+1, inc_lines.begin(), inc_lines.end());
		}
	}
};


void begin_preprocessing() {
	int l_index = 0;
	std::vector<std::string> lines_copy{lines};
	for (std::string l : lines_copy) {
		line_number++;
		l = trim(l);
		_ltoks = split(l);
		_us_ltoks = unspaced(_ltoks);
		
		if (!l.empty()) {
			while_us_find_token("%", 0, 3, [&](TokIt tok_it) {
				token_function::macro(tok_it);
				lines.erase(lines.begin()+l_index);
			});
	  	} else {
	  		lines.erase(lines.begin()+l_index);
	  	}
		l_index++;
	}
}
