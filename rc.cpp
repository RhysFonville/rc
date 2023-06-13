#include <algorithm>
#include <clocale>
#include <iostream>
#include <fstream>
#include <string>
#include <utility>
#include "Token.h"

#define NULL_REG registers.back()

#define ASSERT_NULL_TOKS(min_offset, max_offset) \
for (std::vector<std::string>::iterator it = tok_it-min_offset; it != tok_it+max_offset+1; it++) { \
	if (it == nullptr) { \
		clog::error("Expecting a token", line_number); \
	} \
}

#define ASSERT_NULL_TOK(offset) ASSERT_NULL_TOKS(offset, offset)

#define WHILE_FIND_TOKEN(tok) \
	tok_it = _ltoks.begin(); \
	while (true) { \
		tok_it = remove_constness(_ltoks, find_tok(_ltoks, tok, tok_it)); \
		if (tok_it != _ltoks.end())
#define WHILE_FIND_TOKEN_END \
		else { \
			break; \
		} \
		tok_it++; \
	}
#define WHILE_US_FIND_TOKEN(tok) \
	tok_it = _us_ltoks.begin(); \
	while (true) { \
		tok_it = remove_constness(_us_ltoks, find_tok(_us_ltoks, tok, tok_it)); \
		if (tok_it != _us_ltoks.end())

struct Register {
	std::string name;
	bool occupied;
	
	Register(const std::string &name) : name('%' + name), occupied(false) { }
	Register() : name("%"), occupied(false) { }

	bool operator==(const Register &reg) const noexcept {
		return (
			name == reg.name &&
			occupied == reg.occupied
		);
	}

	bool operator!=(const Register &reg) const noexcept {
		return (
			name != reg.name ||
			occupied != reg.occupied
		);
	}
};

std::vector<std::string> _ltoks;
std::vector<std::string> _us_ltoks;

std::vector<Register> registers = {
	{"rbx"}, {"rcx"}, {"rsp"}, {"rbp"}, {"r11"}, {"r12"}, {"r13"}, {"r14"}, {"r15"},
	{"rax"}, {"rdi"}, {"rsi"}, {"rdx"}, {"r10"}, {"r8"}, {"r9"} // ARGUMENT REGISTERS
};

std::vector<std::string> out;

using uchar = unsigned char;

constexpr char* const ws = " \t\n\r\f\v";

const std::string SYS_READ  = "$0";
const std::string SYS_WRITE = "$1";
const std::string SYS_EXIT  = "$60";
const std::string STDIN     = "$0";
const std::string STDOUT    = "$1";

namespace clog {
	void out(const std::string &str, size_t line = -1) noexcept {
		std::cout << str << std::endl;
	}
	void warn(const std::string &str, size_t line) noexcept {
		std::cerr << (line != -1 ? "LINE: " + std::to_string(line) : "") << "WARNING: " << str << std::endl;
	}
	void error(const std::string &str, size_t line) noexcept {
		std::cerr << (line != -1 ? "LINE: " + std::to_string(line) : "") << "ERROR: " << str << std::endl;
	}
	void note(const std::string &str, size_t line = -1) noexcept {
		std::cout << "NOTE: " << str << std::endl;
	}
}

// trim from end of string (right)
std::string rtrim(std::string s, const char* t = ws) {
	s.erase(s.find_last_not_of(t) + 1);
	return s;
}

// trim from beginning of string (left)
std::string ltrim(std::string s, const char* t = ws) {
	s.erase(0, s.find_first_not_of(t));
	return s;
}

// trim from both ends of string (right then left)
std::string trim(std::string s, const char* t = ws) {
	if (!s.empty())
		return ltrim(rtrim(s, t), t);
	else
		return "";
}

// bool string_is_wrapped(std::string str, size_t substr_begin, size_t substr_end, char c) {
// 	if (str.size() > 1) {
// 		if (substr_begin != 0) {
// 			if (substr_end != str.size()-1) {tok.insert(tok.begin()+1, '')
// 				if (str[substr_begin-1] == c && str[substr_end+1] == c ) {
// 					return true;
// 				} else {
// 					return false;
// 				}
// 			} else {
// 				if (str[substr_begin-1] == c) {
// 					return true;
// 				}
// 			}
// 		} else {
// 			if (substr_end != str.size()-1) {
// 				return false;
// 			} else {
// 				if (str[substr_end+1] == c) {
// 					return true;
// 				} else {
// 					return false;
// 				}
// 			}
// 		}
// 	} else {
// 		return false;
// 	}
// 	return false;
// }

std::vector<std::string> split(const std::string &str) { // IT WORKS!! WOW!!
	std::vector<std::string> ret;
	std::vector<bool> ret_is_token;

	ret.push_back(str);
	ret_is_token.push_back(false);

	for (const Token &token : tokens) {
		int j = 0;
		for (int i = 0; i < ret.size(); i++) {
			if (!ret_is_token[i]) {
				std::string chunk = ret[i];
				size_t token_index = chunk.find(token.token);
				if (token_index != std::string::npos) {
					ret.erase(ret.begin()+i);
					ret_is_token.erase(ret_is_token.begin()+i);

					std::string sub = chunk.substr(0, token_index);
					if (!sub.empty()) {
						ret.insert(ret.begin()+i++, chunk.substr(0, token_index));
						ret_is_token.insert(ret_is_token.begin()+(i-1), false);
					}

					ret.insert(ret.begin()+i++, token.token);
					ret_is_token.insert(ret_is_token.begin()+(i-1), true);

					int begin = token_index+token.token.size();
					sub = chunk.substr(begin);
					if (!sub.empty()) {
						ret.insert(ret.begin()+i, sub);
						ret_is_token.insert(ret_is_token.begin()+i, false);
					}
				}
			}
			i = j;
			j++;
		}
	}
	
	std::vector<std::string>::const_iterator find_quote;
	std::vector<std::string>::const_iterator quote_begin = ret.begin();
	while (true) {
		find_quote = std::find(quote_begin, ret.cend(), "\"");
		if (find_quote != ret.end()) {
				std::vector<std::string>::const_iterator end = ret.end();
				std::vector<std::string>::const_iterator find_end_quote = std::find(find_quote+1, end, "\"");
				
				std::string str = "";
				for (auto cit = find_quote; cit != find_end_quote+1; ++cit) {
					str += *cit;
				}
				ret.erase(find_quote, find_end_quote+1);
				ret.insert(find_quote, str);
				
				quote_begin = find_end_quote;
		} else {
			break;
		}
	}

	return ret;
}

std::vector<std::string> unspaced(const std::vector<std::string> &vec) {
	std::vector<std::string> ret;
	for (const std::string &str : vec) {
		if (str != " ") ret.push_back(str);
	}
	return ret;
}

std::vector<std::string>::const_iterator find_tok(const std::vector<std::string> &ltoks, const std::string &str,
				const std::vector<std::string>::const_iterator &begin) {
	return std::find(begin, ltoks.end(), str);
}

size_t from_it(const std::vector<std::string> &vec, const std::vector<std::string>::const_iterator &it) {
	return std::distance(vec.begin(), it);
}

std::vector<std::string> replace_toks(std::vector<std::string> toks, size_t begin, size_t end, const std::string &str) {
	toks.erase(toks.begin()+begin, toks.begin()+end+1);
	toks.insert(toks.begin()+begin, str);
	
	return toks;
}

std::vector<std::string> replace_toks(const std::vector<std::string> &toks, std::vector<std::string>::const_iterator begin,
		std::vector<std::string>::const_iterator end, const std::string &str) {
	return replace_toks(toks, from_it(toks, begin), from_it(toks, end), str);
}

std::vector<std::string> replace_tok(std::vector<std::string> toks, size_t i, const std::string &str) {
	toks[i] = str;
	return toks;
}

std::vector<std::string> replace_tok(std::vector<std::string> toks, std::vector<std::string>::const_iterator i, const std::string &str) {
	toks.erase(toks.begin()+from_it(toks, i), toks.begin()+1);
	toks.insert(toks.begin()+from_it(toks, i), str);
	return toks;
}

void commit(const std::vector<std::string> &new_vec) {
	_ltoks = new_vec;
	if (find_tok(new_vec, " ", new_vec.begin()) ==	new_vec.end()) {
		_us_ltoks = new_vec;
	} else {
		_us_ltoks = unspaced(_ltoks);
	}
}

template <typename Container, typename ConstIterator>
typename Container::iterator remove_constness(Container& c, ConstIterator it) {
    return c.erase(it, it);
}

std::string combine_toks(const std::vector<std::string> &toks, const std::vector<std::string>::const_iterator &begin,
				const std::vector<std::string>::const_iterator &end) {
	std::string ret = "";
	for (auto it = begin; it != end; it++) {
		ret += *it;
	}
	return ret;
}

Register & get_available_register() {
	for (Register &reg : registers) {
		if (!reg.occupied) return reg;
	}
	return NULL_REG;
}

Register & get_register(const std::string &str) {
	for (Register &reg : registers) {
		if (reg.name == str) return reg;
	}
	
	return NULL_REG;
}

std::string get_string_literal(const std::vector<std::string> &toks, std::vector<std::string>::iterator index, bool with_quotes = true) { // Not actually used but I'm keeping it
	std::string ret = "";
	for (index++; *index != "\""; index++) {
		ret += *index;
	}
	if (with_quotes) {
		ret = toks[0] + ret;
		ret += *index;
	}

	return ret;
}

bool is_number(const std::string &s) {
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

std::string prep_asm_str(std::string str) {
	if (is_number(trim(str))) {
		str = '$' + str;
	}

	return str;
}

namespace specfic_actions {
	void math_token(std::vector<std::string>::iterator tok_it) {
		std::string cmd;
		if (*tok_it == "*") {
			cmd = "mul";
		} else if (*tok_it == "/") {
			cmd = "div";
		} else if (*tok_it == "+") {
			cmd = "add";
		} else if (*tok_it == "-") {
			cmd = "sub";
		} else if (*tok_it == "%") {
			cmd = "% not supported yet";
		}
		if (cmd == "add" || cmd == "sub") {
			Register &lhs = get_available_register();
			lhs.occupied = true;
			Register &rhs = get_available_register();
			rhs.occupied = true;
			out.push_back("movq " + prep_asm_str(*(tok_it-1)) +  ", " + lhs.name  + '\n');
			out.push_back("movq " + prep_asm_str(*(tok_it+1)) +  ", " + rhs.name + '\n');
			out.push_back(cmd + ' ' + lhs.name + ", " + rhs.name + '\n');
			commit(replace_toks(_us_ltoks, tok_it-1, tok_it+1, lhs.name));
			lhs.occupied = true;
			rhs.occupied = false;
		} else if (cmd == "mul" || cmd == "div") {
			Register &lhs = get_available_register();
			lhs.occupied = true;
			Register &rhs = get_available_register();
			lhs.occupied = true;
			
			out.push_back("movq " + prep_asm_str(*(tok_it-1)) + ", " + lhs.name + '\n');
			out.push_back("movq " + prep_asm_str(*(tok_it+1)) + ", " + rhs.name + '\n');
			out.push_back(cmd + ' ' + lhs.name + '\n');
			commit(replace_toks(_us_ltoks, tok_it-1, tok_it+1, lhs.name));
			lhs.occupied = true;
			rhs.occupied = false;
		}
	}
}

int main(int argc, char *argv[]) {
	std::ifstream read;
	read.open(argv[1], std::ifstream::in);
	std::ofstream write;
	write.open("rcout.s", std::ofstream::out | std::ios::trunc);
	
	out.push_back(".text\n");
	out.push_back(".globl _start\n");
	out.push_back("_start:\n");	
	
	registers.push_back(Register());
	
	std::vector<std::string>::iterator tok_it;

	size_t line_number = 0;
	std::string l;
	while (std::getline(read, l)) {
		line_number++;
		l = trim(l);
		if (!l.empty()) {
			_ltoks = split(l);
			_us_ltoks = unspaced(_ltoks);

			// no clue why I have to do any of this
			// data_marker = 0;
			// bss_marker = 0;
			// while (out[data_marker] != "section .data\n") {
			// 	data_marker++;
			// }
 			// while (out[bss_marker] != "section .bss\n") {
			// 	bss_marker++;
			// }

			//for (std::vector<std::string>::iterator it = _us_ltoks.begin(); it != _us_ltoks.end(); it++) {
			//	if (get_var(*it) != Variable()) {
			//		it->insert(it->begin(), '[');
			//		it->insert(it->end(), ']');
			//	}
			//}
			//commit(_us_ltoks);
			
			WHILE_US_FIND_TOKEN("//") {
				int i = std::distance(_us_ltoks.begin(), tok_it);
				while ( i < _us_ltoks.size()) {
					_us_ltoks.erase(_us_ltoks.begin()+i);
				}
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("~") {
				commit(replace_tok(_us_ltoks, tok_it, "rax"));
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("^") {
				//_us_ltoks.erase(tok_it);
				//tok_it->erase(tok_it->begin());
				//tok_it->erase(tok_it->end()-1);
				_us_ltoks.erase(tok_it);
				tok_it->insert(tok_it->begin(), '(');
				tok_it->insert(tok_it->end(), ')');
				commit(_us_ltoks);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("*") {
				specfic_actions::math_token(tok_it);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("/") {
				specfic_actions::math_token(tok_it);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("%") {
				specfic_actions::math_token(tok_it);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("+") {
				specfic_actions::math_token(tok_it);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("-") {
				specfic_actions::math_token(tok_it);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("def ") {
				std::string name = *(tok_it+2) + ": \n";
				out.insert(out.begin(), name);

				std::string def_type = "";
				std::string type = *(tok_it+1);
				if (type == "int") {
					def_type = ".word ";
				} else if (type == "ch") {
					def_type = ".asciz ";
				} else if (type == "wd") {
					def_type = ".long ";
				}

				if (tok_it+3 == out.end() || (tok_it+3)->empty()) {
					def_type += "1";
				} else {
					def_type += combine_toks(_us_ltoks, tok_it+3, _us_ltoks.end());
				}

				out.insert(out.begin(), def_type+'\n');
			} WHILE_FIND_TOKEN_END
			/*WHILE_US_FIND_TOKEN("res") {
				std::string str = *(tok_it+2)+": ";
				std::string type = *(tok_it+1);
				if (type == "int") {
					str += ".word ";
				} else if (type == "ch") {
					str += ".asciz ";
				} else if (type == "long") {
					str += ".long ";
				}

				if (tok_it+3 == out.end() || (tok_it+3)->empty()) {
					str += "1";
				} else {
					str += *(tok_it+3);
				}

				out.insert(out.begin(), str+'\n');
			} WHILE_FIND_TOKEN_END*/
			WHILE_US_FIND_TOKEN("=") {
				std::string rhs = *(tok_it+1);
				if (Register rhs_reg = get_register(*(tok_it+1)); rhs_reg != NULL_REG) {
					rhs_reg.occupied = false;
				} else {
					Register reg = get_available_register();
					out.push_back("movq " + prep_asm_str(rhs) + ", " + reg.name + '\n');
					rhs = reg.name;
					reg.occupied = false;
				}
				
				out.push_back("movq " + prep_asm_str(rhs) + ", " + prep_asm_str(*(tok_it-1)) + '\n');
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN(">") {
				if (*(tok_it+1) == "w") { // WRITE
					out.push_back("movq " + SYS_WRITE + ", %rax" + '\n');
					out.push_back("movq " + prep_asm_str(*(tok_it+2)) + ", %rdi" + '\n');
					out.push_back("movq " + prep_asm_str(*(tok_it+3)) + ", %rsi" + '\n');
					out.push_back("movq " + prep_asm_str(*(tok_it+4)) + ", %rdx" + '\n');
					out.push_back("syscall\n");
					
					get_register("rax").occupied = false;
					get_register("rdi").occupied = false;
					get_register("rsi").occupied = false;
					get_register("rdx").occupied = false;
				} else if (*(tok_it+1) == "r") { // READ
					out.push_back("movq " + SYS_READ + ", %rax" + '\n');
					out.push_back("movq " + prep_asm_str(*(tok_it+2)) + ", %rdi" + '\n');
					out.push_back("movq " + prep_asm_str(*(tok_it+3)) + ", %rsi" + '\n');
					out.push_back("movq " + prep_asm_str(*(tok_it+4)) + ", %rdx" + '\n');
					out.push_back("syscall\n");

					get_register("rax").occupied = false;
					get_register("rdi").occupied = false;
					get_register("rsi").occupied = false;
					get_register("rdx").occupied = false;
				} else if (*(tok_it+1) == "e") { // EXIT
					out.push_back("movq " + SYS_EXIT + ", %rax" + '\n');
					out.push_back("movq " + prep_asm_str(*(tok_it+2)) + ", %rdi" + '\n');
					out.push_back("syscall\n");

					get_register("rax").occupied = false;
					get_register("rdi").occupied = false;
				}
			} WHILE_FIND_TOKEN_END
		}
	}

	for (const std::string &str : out) {
		write << str;
	}
	
	read.close();
	write.close();

	system(((std::string)"as rcout.s -o rcout.o && ld rcout.o && ./a.out").c_str());
	//system(((std::string)"rm " + argv[2] + ".asm").c_str());
	system(((std::string)"rm rcout.o").c_str());

	std::cout << std::endl;
	
	return 0;
}
