#include <fstream>
#include "util.h"

namespace token_function {
	void macro(TokIt tok_it) {
		if (*(tok_it+1) == "inc") {
			if (*(tok_it+2) != "\"" || *(tok_it+4) != "\"") {
				message::error("Include macro parameter must be wrapped in double quotes in a valid manner.");
			}
			
			std::ifstream in(*(tok_it+3));
			
			std::vector<std::string> inc_lines = { };
			std::string l;
			while (std::getline(in, l)) {
				inc_lines.push_back(l);
			}
			
			lines.insert(lines.begin()+line_number, inc_lines.begin(), inc_lines.end());
		}
	}
};


void begin_preprocessing() {
	for (; line_number-1 < lines.size(); line_number++) {
		lines[line_number-1] = trim(lines[line_number-1]);
		_ltoks = split(lines[line_number-1]);
		_us_ltoks = unspaced(_ltoks);
		
		if (!lines[line_number-1].empty()) {
			while_us_find_token("%", 0, 3, [&](TokIt tok_it) {
				token_function::macro(tok_it);
				lines.erase(lines.begin()+line_number-1);
				line_number--;
			});
	  	} else {
	  		lines.erase(lines.begin()+line_number-1);
			line_number--;
	  	}
	}
}
