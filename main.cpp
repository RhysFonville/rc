#include <algorithm>
#include <clocale>
#include <iostream>
#include <fstream>
#include <string>
#include <utility>
#include "Token.h"

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
#define WHILE_USR_FIND_TOKEN(tok) \
	tok_it = _us_rltoks.begin(); \
	while (true) { \
		tok_it = remove_constness(_us_rltoks, find_tok(_us_rltoks, tok, tok_it)); \
		if (tok_it != _us_rltoks.end())

struct Register {
	std::string name;
	std::string value;
	bool occupied;
	
	Register(const std::string &name) : name(name), value(""), occupied(false) { }
	Register() : name(""), value(""), occupied(false) { }
};

struct Variable {
	std::string name;
	std::string type;
	std::string value;
	int address;

	Variable(const std::string &name, const std::string &type, std::string value = "")
			: name(name), type(type), value(value), address(0) { }
	Variable() : name(""), type(""), value(""), address(0) { }

	bool operator==(const Variable &var) const noexcept {
		return (name == var.name &&
				value == var.value &&
				address == var.address);
	}

	bool operator!=(const Variable &var) const noexcept {
		return (name != var.name ||
				value != var.value ||
				address != var.address);
	}
};

std::vector<std::string> _ltoks;
std::vector<std::string> _us_ltoks;
std::vector<std::string> _us_rltoks;

std::vector<Variable> vars;

std::vector<Register> registers = { {"rax"}, {"rdi"}, {"rsi"}, {"rdx"}, {"r10"}, {"r8"}, {"r9"} };

std::vector<std::string> out;

using uchar = unsigned char;

constexpr char* ws = " \t\n\r\f\v";

const std::string SYS_READ  = "0";
const std::string SYS_WRITE = "1";
const std::string SYS_EXIT  = "60";
const std::string STDIN     = "0";
const std::string STDOUT    = "1";

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

bool string_is_wrapped(std::string str, size_t substr_begin, size_t substr_end, char c) {
	if (str.size() > 1) {
		if (substr_begin != 0) {
			if (substr_end != str.size()-1) {
				if (str[substr_begin-1] == c && str[substr_end+1] == c ) {
					return true;
				} else {
					return false;
				}
			} else {
				if (str[substr_begin-1] == c) {
					return true;
				}
			}
		} else {
			if (substr_end != str.size()-1) {
				return false;
			} else {
				if (str[substr_end+1] == c) {
					return true;
				} else {
					return false;
				}
			}
		}
	} else {
		return false;
	}
	return false;
}

std::vector<std::string> split(const std::string &str) {
	std::vector<std::string> ret;

	const size_t str_end = str.length()-1;

	size_t substr_begin = 0;
	size_t substr_end = str_end;

	bool found_token;
	do {
        found_token = false; 
		size_t pos = std::string::npos;
		Token token_found;
		for (const Token &token : tokens) {
			size_t find_pos = str.find(token.token, substr_begin);
			if (find_pos != std::string::npos) {
				if (find_pos < pos) {
					token_found = token;
					pos = find_pos;
				}
			}
		}
		
		if (pos != std::string::npos) {
			substr_end = pos;
			if (substr_end == substr_begin)
				substr_end++;
				
			//if ((token_found.is_word && !string_is_wrapped(str, substr_begin, substr_end, ' ')) ||
			//	!token_found.is_word) {
				ret.push_back(str.substr(substr_begin, substr_end-substr_begin));
			//}
			substr_begin = substr_end;
			substr_end = str_end+1;
			
			found_token = true;
		}
	} while (found_token);

	if (std::string leftover = str.substr(substr_begin); !leftover.empty()) {
		ret.push_back(leftover);
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

std::vector<std::string> replace_vars(std::vector<std::string> ltoks) {
	size_t i = 0;
	for (const std::string &tok : ltoks) {
		for (const Variable &variable : vars) {
			if (tok == variable.name) {
				if (i > 0) {
					if (ltoks[i-1] == "^" || (*tok.begin() == '[' && *tok.end()-1 == ']')) {
						continue;
					}
				}
				ltoks = replace_toks(ltoks, i, i, variable.value);
			}
		}
		i++;
	}
	return ltoks;
}

void commit(const std::vector<std::string> &new_vec) {
	_ltoks = new_vec;
	if (find_tok(new_vec, " ", new_vec.begin()) ==	new_vec.end()) {
		_us_ltoks = new_vec;
	} else {
		_us_ltoks = unspaced(_ltoks);
	}
	_us_rltoks = replace_vars(_us_ltoks);
}

Variable get_var(std::string name) {
	if (*(name.begin()) == '[' && *(name.end()-1) == ']') {
		name = name.substr(1, name.size()-2);
	}		
	for (const Variable &var : vars) {
		if (name == var.name) {
			return var;
		}
	}
	return Variable(); 
}

Register get_register(const std::string &name) {
	for (const Register &reg : registers) {
		if (reg.name == name) return name;
	}
	return Register();
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
			if (get_var(*(tok_it-1)) != Variable()) {
				out.push_back("mov rax, " + *(tok_it-1) + '\n');
				if (get_var(*(tok_it+1)) != Variable()) {
					out.push_back(cmd + " rax, " + *(tok_it+1) + '\n');
				} else {
					out.push_back(cmd + " rax, " + *(tok_it+1) + '\n');
				}
				commit(replace_toks(_us_ltoks, tok_it-1, tok_it+1, "rax"));
			} else {
				out.push_back("mov rax, " + *(tok_it-1) + '\n');
				if (get_var(*(tok_it+1)) != Variable()) {
					out.push_back("mov rbx, " + *(tok_it+1) + '\n');
					out.push_back(cmd + " rax, rbx\n");
				} else {
					out.push_back(cmd + " rax, " + *(tok_it+1) + '\n');
					commit(replace_toks(_us_ltoks, tok_it-1, tok_it+1, std::to_string(stoi(*(tok_it-1)) * stoi(*(tok_it+1)))));
				}
			}
		} else if (cmd == "mul" || cmd == "div") {
			if (get_var(*(tok_it-1)) != Variable()) { }
				
		}
	}
}

int main(int argc, char *argv[]) {
	std::ifstream read;
	read.open(argv[1], std::ifstream::in);
	std::ofstream write;
	write.open(std::string(argv[2]) + ".asm", std::ofstream::out | std::ios::trunc);
	
	out.push_back("section .data\n");
	out.push_back("section .bss\n");
	out.push_back("global _start\n");
	out.push_back("section .text\n");
	out.push_back("_start:\n");	
	
	std::vector<std::string>::iterator data_marker = out.begin();
	std::vector<std::string>::iterator bss_marker = out.begin()+1;
	
	std::vector<std::string>::iterator tok_it;

	size_t line_number = 0;
	std::string l;
	while (std::getline(read, l)) {
		line_number++;
		l = trim(l);
		if (!l.empty()) {
			_ltoks = split(l);
			_us_ltoks = unspaced(_ltoks);
			_us_rltoks = replace_vars(_us_ltoks);

			//for (std::vector<std::string>::iterator it = _us_ltoks.begin(); it != _us_ltoks.end(); it++) {
			//	if (get_var(*it) != Variable()) {
			//		it->insert(it->begin(), '[');
			//		it->insert(it->end(), ']');
			//	}
			//}
			//commit(_us_ltoks);
			
			WHILE_US_FIND_TOKEN("~") {
				*tok_it = "rax";
				commit(_us_ltoks);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("^") {
				//_us_ltoks.erase(tok_it);
				//tok_it->erase(tok_it->begin());
				//tok_it->erase(tok_it->end()-1);
				_us_ltoks.erase(tok_it);
				tok_it->insert(tok_it->begin(), '[');
				tok_it->insert(tok_it->end(), ']');
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
			WHILE_US_FIND_TOKEN("exit") {
				out.push_back("mov rax, " + SYS_EXIT + '\n');
				out.push_back("mov rdi, " + *(tok_it+1) + '\n');
				out.push_back("syscall\n");
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("def") {
				Variable var;
				std::string str = *(tok_it+2)+": ";
				std::string type = *(tok_it+1);
				if (type == "by") {
					str += "db ";
					var.type = "byt
				} else if (type == "int") {
					str += "dw ";
				}
				if (tok_it+3 == out.end() || (tok_it+3)->empty()) {
					str += "1";
				} else {
					str += combine_toks(_us_ltoks, tok_it+3, _us_ltoks.end());
				}
				vars.push_back(Variable(*(tok_it+2), *(tok_it+3)));
				out.insert(data_marker, str+'\n');
				data_marker++;
				bss_marker++;
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("res") {
				std::string str = *(tok_it+2)+": ";
				std::string type = *(tok_it+1);
				if (type == "ch") {
					str += "resb ";
				} else if (type == "wd") {
					str += "resw ";
				}
				if (tok_it+3 == out.end() || (tok_it+3)->empty()) {
					str += "1";
				} else {
					str += *(tok_it+3);
				}
				vars.push_back(Variable(*(tok_it+2), ""));
				out.insert(bss_marker+1, str+'\n');
				bss_marker++;
				data_marker++;
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("=") {
				out.push_back("mov " + *(tok_it-1) + ", " + *(tok_it+1) + '\n'); 
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("<") {
				out.push_back("mov rax, " + SYS_WRITE + '\n');
				out.push_back("mov rdi, " + *(tok_it-1) + '\n');
				out.push_back("mov rsi, " + *(tok_it+1) + '\n');
				out.push_back("mov rdx, " + *(tok_it+2) + '\n');
				out.push_back("syscall\n");
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN(">") {
				out.push_back("mov rax, " + SYS_READ + '\n');
				out.push_back("mov rdi, " + *(tok_it-1) + '\n');
				out.push_back("mov rsi, " + *(tok_it+1) + '\n');
				out.push_back("mov rdx, " + *(tok_it+2) + '\n');
				out.push_back("syscall\n");
			} WHILE_FIND_TOKEN_END
		}	
	}

	for (const std::string &str : out) {
		write << str;
	}
	
	read.close();
	write.close();

	system(((std::string)"nasm -felf64 " + argv[2] + ".asm && ld " + argv[2] + ".o && ./a.out").c_str());
	//system(((std::string)"rm " + argv[2] + ".asm").c_str());
	system(((std::string)"rm " + argv[2] + ".o").c_str());	
	
	return 0;
}

