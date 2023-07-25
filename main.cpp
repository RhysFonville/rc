#include <algorithm>
#include <clocale>
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>
#include "Token.h"

#define NULL_REG registers.back()

#define WHILE_FIND_TOKEN(tok) \
	if (std::find(disallowed_toks.begin(), disallowed_toks.end(), #tok) == disallowed_toks.end()) { \
		tok_it = _ltoks.begin(); \
		while (true) { \
			tok_it = remove_constness(_ltoks, find_tok(_ltoks, tok, tok_it)); \
			if (tok_it != _ltoks.end())

#define WHILE_FIND_TOKEN_END \
			else { \
				break; \
			} \
			tok_it++; \
		} \
	}

#define WHILE_US_FIND_TOKEN(tok) \
	if (std::find(disallowed_toks.begin(), disallowed_toks.end(), #tok) == disallowed_toks.end()) { \
		tok_it = _us_ltoks.begin(); \
		while (true) { \
			tok_it = remove_constness(_us_ltoks, find_tok(_us_ltoks, tok, tok_it)); \
			if (tok_it != _us_ltoks.end())

#define WHILE_FIND_TOKENS(toks) \
	if (std::find(disallowed_toks.begin(), disallowed_toks.end(), #toks) == disallowed_toks.end()) { \
		tok_it = _ltoks.begin(); \
		while (true) { \
			tok_it = remove_constness(_ltoks, find_first_tok(_ltoks, toks, tok_it)); \
			if (tok_it != _ltoks.end())

#define WHILE_US_FIND_TOKENS(toks) \
	if (std::find(disallowed_toks.begin(), disallowed_toks.end(), #toks) == disallowed_toks.end()) { \
		tok_it = _us_ltoks.begin(); \
		while (true) { \
			tok_it = remove_constness(_us_ltoks, find_first_tok(_us_ltoks, toks, tok_it)); \
			if (tok_it != _us_ltoks.end())

using TokIt = std::vector<std::string>::iterator;

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
	{"rbx"}, {"rcx"}, {"r11"}, {"r12"}, {"r13"}, {"r14"}, {"r15"},
	{"rax"}, {"rdi"}, {"rsi"}, {"rdx"}, {"r10"}, {"r8"}, {"r9"}, // ARGUMENT REGISTERS
	{"rsp"}, {"rbp"} // STACK REGISTERS
};

namespace types {
	const std::vector<std::string> types = { "int", "str", "nstr", "lng", "sht", "byt" };
	const std::vector<std::string> asm_types = { ".word", ".asciz", ".ascii", ".long", ".short", ".byte" };
	const std::vector<unsigned char> sizes = { 4, 8, 8, 8, 2, 1 };
};
const std::vector<std::string> math_symbols = { "*", "/", "+", "-", "%" };

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
		std::cerr << (line != -1 ? "LINE: " + std::to_string(line) : "") << " ERROR: " << str << std::endl;
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
// 			if (substr_end != str.size()-1) {
// 				if (str[substr_begin-1] == c && str[substr_end+1] == c) {
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

template <typename Container, typename ConstIterator>
typename Container::iterator remove_constness(Container& c, ConstIterator it) {
    return c.erase(it, it);
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

std::vector<std::string>::const_iterator find_first_tok(const std::vector<std::string> &ltoks, const std::vector<std::string> &toks_to_find,
				const std::vector<std::string>::const_iterator &begin) {
	for (const std::string &tok : toks_to_find) {
		if (auto it = std::find(begin, ltoks.end(), tok); it != ltoks.end()) return it;
	}

	return ltoks.end();
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

std::vector<std::string> replace_tok(const std::vector<std::string> &toks, std::vector<std::string>::const_iterator it, const std::string &str) {
	return replace_tok(std::vector<std::string>(toks), from_it(toks, it), str);
}

void commit(const std::vector<std::string> &new_vec) {
	_ltoks = new_vec;
	if (find_tok(new_vec, " ", new_vec.begin()) ==	new_vec.end()) {
		_us_ltoks = new_vec;
	} else {
		_us_ltoks = unspaced(_ltoks);
	}
}


std::string combine_toks(const std::vector<std::string>::const_iterator &begin, const std::vector<std::string>::const_iterator &end) {
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

Register & get_register(std::string str) {
	for (Register &reg : registers) {
		if ((reg.name == str) || (reg.name.substr(1) == str)) return reg;
	}
	
	return NULL_REG;
}

std::string get_string_literal(const std::vector<std::string> &toks, TokIt index, bool with_quotes = true) { // Not actually used but I'm keeping it
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
	if (get_register(str) == NULL_REG && str.back() != ')') {
		str = '$' + str;
	}

	return str;
}

namespace token_function {
	void dereference(TokIt tok_it) {
		_us_ltoks.erase(tok_it);
		tok_it->insert(tok_it->begin(), '(');
		tok_it->insert(tok_it->end(), ')');
		commit(_us_ltoks);
	}

	void address_of(TokIt tok_it) {
		Register reg = get_available_register();
		out.push_back("leaq " + *(tok_it+1) + ", " + reg.name + '\n');
		
		commit(replace_toks(_us_ltoks, tok_it, tok_it+1, reg.name));
	}

	void math(TokIt tok_it) {
		std::string cmd;
		if (*tok_it == "*") {
			cmd = "mulq";
		} else if (*tok_it == "/") {
			cmd = "divq";
		} else if (*tok_it == "+") {
			cmd = "addq";
		} else if (*tok_it == "-") {
			cmd = "subq";
		} else if (*tok_it == "%") {
			cmd = "% not supported yet";
		}
		if (cmd == "addq" || cmd == "subq") {
			Register &lhs = get_available_register();
			lhs.occupied = true;
			Register &rhs = get_available_register();
			rhs.occupied = true;
			out.push_back("movq " + prep_asm_str(*(tok_it-1)) +  ", " + lhs.name  + '\n');
			out.push_back("movq " + prep_asm_str(*(tok_it+1)) +  ", " + rhs.name + '\n');
			out.push_back(cmd + ' ' + rhs.name + ", " + lhs.name + '\n');
			commit(replace_toks(_us_ltoks, tok_it-1, tok_it+1, lhs.name));
			lhs.occupied = true;
			rhs.occupied = false;
		} else if (cmd == "mulq" || cmd == "divq") {
			Register &lhs = get_register("rax");
			lhs.occupied = true;
			Register &rhs = get_register("rdx");
			lhs.occupied = true;
			
			out.push_back("movq " + prep_asm_str(*(tok_it-1)) + ", " + lhs.name + '\n');
			out.push_back("movq " + prep_asm_str(*(tok_it+1)) + ", " + rhs.name + '\n');
			out.push_back(cmd + ' ' + rhs.name + '\n');
			commit(replace_toks(_us_ltoks, tok_it-1, tok_it+1, lhs.name));
			lhs.occupied = true;
			rhs.occupied = false;
		}
	}

	void variable_declaration(TokIt tok_it, std::string current_function, std::vector<std::string> &variable_names, std::vector<int> &variable_sizes, std::vector<int> &variable_stack_locations, int &current_stack_size) {
		size_t type_vec_index = from_it(types::types, std::find(types::types.begin(), types::types.end(), *tok_it)); // Very long, it just gets the asm_type from tok_it
		
		if (current_function.empty()) {
			out.insert(out.begin(), ".globl " + *(tok_it+1) + '\n');
			out.insert(out.begin()+1, ".align " + std::to_string(types::sizes[type_vec_index]) + '\n'); // Align the same size as type
			out.insert(out.begin()+2, ".type " + *(tok_it+1) + ", @object\n");
			out.insert(out.begin()+3, *(tok_it+1) + ":\n");
			
			std::string def_type = types::asm_types[type_vec_index];
			def_type += ' ';

			if (tok_it+2 == out.end() || (tok_it+2)->empty()) {
				def_type += '0';
			} else {
				def_type += combine_toks(tok_it+2, _us_ltoks.end());
			}

			out.insert(out.begin()+4, def_type+'\n');
			variable_names.push_back(*(tok_it+1));
			variable_sizes.push_back(types::sizes[type_vec_index]);
			variable_stack_locations.push_back(-1);
		} else {
			
			current_stack_size += types::sizes[type_vec_index];
			//out.push_back("subq $" + std::to_string(types::sizes[type_vec_index]) + ", %rsp\n");
			out.push_back("movq " + prep_asm_str(*(tok_it+2)) + ", -" +  std::to_string(current_stack_size) + "(%rbp)\n");
			variable_names.push_back(*(tok_it+1));
			variable_sizes.push_back(types::sizes[type_vec_index]);
			variable_stack_locations.push_back(-current_stack_size);
		}
	}

	void equals(TokIt tok_it) {
		std::string rhs = *(tok_it+1);
		
		std::cout << rhs << std::endl;
		std::cout << *(tok_it-1) << std::endl;
		
		if (Register rhs_reg = get_register(*(tok_it+1)); rhs_reg != NULL_REG) {
			rhs_reg.occupied = false;
		} else {
			Register reg = get_available_register();
			out.push_back("movq " + prep_asm_str(rhs) + ", " + reg.name + '\n');
			rhs = reg.name;
			reg.occupied = false;
		}
		
		out.push_back("movq " + prep_asm_str(rhs) + ", " + prep_asm_str(*(tok_it-1)) + '\n');
	}

	void base_functions(TokIt tok_it) {
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
	}
	void function_declaration(TokIt tok_it, std::vector<std::string> &functions, std::vector<int> &stack_sizes, std::string &current_function) {
		std::string func_name = *(tok_it+1);
		out.push_back(".globl " + func_name + '\n');
		out.push_back(".type " + func_name + ", @function\n");
		out.push_back(func_name + ":\n");
		out.push_back("pushq %rbp\n");
		out.push_back("movq %rsp, %rbp\n");
		out.push_back("subq $16, %rsp\n");

		functions.push_back(func_name);
		stack_sizes.push_back(0);
		current_function = func_name;
	}
	void function_call(TokIt tok_it) {
		out.push_back("movq $0, %rax\n");
		out.push_back("call " + *tok_it + '\n');
		//for (std::string tok : _us_ltoks) {
		//	std::cout << tok << ' ';
		//}
		//std::cout << std::endl;
		commit(replace_tok(_us_ltoks, tok_it, "%rax"));
		//for (std::string tok : _us_ltoks) {
		//	std::cout << tok << ' ';
		//}
		//std::cout << std::endl;
	}
	void function_return(TokIt tok_it, const std::vector<std::string> &toks, std::string &current_function) {
		out.push_back("movq " + (tok_it+1 != toks.end() ? *(tok_it+1) : "0") + ", %rax\n");
		out.push_back("leave\n");
		out.push_back("ret\n");
		out.push_back(".size " + current_function + ", .-" + current_function + '\n');
		current_function = "";
	}
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		clog::error("Must input file to compile.", 0);
		return 0;
	}

	std::ifstream read;
	read.open(argv[1], std::ifstream::in);
	std::ofstream write;
	write.open("rcout.s", std::ofstream::out | std::ios::trunc);
	
	std::vector<std::string> disallowed_toks = { };
	
	std::vector<std::string> variable_names = { };
	std::vector<int> variable_sizes = { };
	std::vector<int> variable_stack_locations = { };

	std::vector<std::string> functions = { };
	std::vector<int> current_function_stack_sizes = { };
	std::string current_function = "";

	// --------- MAIN ---------
	out.push_back(".text\n");
	// ------------------------
	
	registers.push_back(Register());
	
	TokIt tok_it;

	size_t line_number = 0;
	std::string l;
	while (std::getline(read, l)) {
		line_number++;
		l = trim(l);
		if (!l.empty()) {
			_ltoks = split(l);
			_us_ltoks = unspaced(_ltoks);
			
			WHILE_US_FIND_TOKEN("//") {
				int i = std::distance(_us_ltoks.begin(), tok_it);
				while ( i < _us_ltoks.size()) {
					_us_ltoks.erase(_us_ltoks.begin()+i);
				}
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("#") {
				token_function::function_declaration(tok_it, functions, current_function_stack_sizes, current_function);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKENS(functions) {
				if (*(tok_it-1) != "#") {
					token_function::function_call(tok_it);
				}
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKENS(types::types) {
				size_t func_vec_index = from_it(functions, std::find(functions.begin(), functions.end(), current_function));
				token_function::variable_declaration(tok_it, current_function, variable_names, variable_sizes, variable_stack_locations, current_function_stack_sizes[func_vec_index]);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKENS(variable_names) {
				size_t vec_index = from_it(variable_names, std::find(variable_names.begin(), variable_names.end(), *tok_it));
				if (variable_stack_locations[vec_index] != -1) {
					commit(replace_tok(_us_ltoks, tok_it, std::to_string(variable_stack_locations[vec_index]) + "(%rbp)"));
				}
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("#>") {
				token_function::function_return(tok_it, _us_ltoks, current_function);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("~") {
				commit(replace_tok(_us_ltoks, tok_it, "rax"));
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("&") {
				token_function::address_of(tok_it);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("^") {
				token_function::dereference(tok_it);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKENS(math_symbols) {
				token_function::math(tok_it);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("=") {
				token_function::equals(tok_it);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN(">") {
				token_function::base_functions(tok_it);
			} WHILE_FIND_TOKEN_END

			disallowed_toks.clear();
		}
	}

	// --------- STICK TO TOP OF FILE ---------
	out.insert(out.begin(), ".data\n");
	out.insert(out.begin(), ".file \"" + std::string(argv[1]) + "\"\n");
	// ----------------------------------------

	for (const std::string &str : out) {
		write << str;
	}
	
	read.close();
	write.close();

	system(((std::string)"as rcout.s -o rcout.o && ld -e main rcout.o && ./a.out").c_str());
	system(((std::string)"rm rcout.o").c_str());

	std::cout << std::endl;
	
	return 0;
}

