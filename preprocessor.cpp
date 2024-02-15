#include "util.h"

namespace token_function {
	void macro(TokIt tok_it) {
		
	}
};


void begin_preprocessing() {
	int l_index = 0;
	for (const std::string &l : lines) {	
		if (!l.empty()) {
			while_us_find_token("%", 0, 2, [&](TokIt tok_it) {
				token_function::macro(tok_it, lines);
			});
	  	} else {
	  		lines.erase(lines.begin()+l_index);
	  	}
		l_index++;
	}
}
