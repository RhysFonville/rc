#include <algorithm>
#include <clocale>
#include <functional>
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <type_traits>
#include <utility>
#include <climits>
#include <optional>
#include "Token.h"

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
	struct Names {
		std::string q;
		std::string l;
		std::string w;
		std::string bh;
		std::string bl;

		bool operator==(const Names &reg) const noexcept {
			return (q == q &&
					l == l &&
					w == w &&
					bh == bh &&
					bl == bl);
		}

		bool operator!=(const Names &reg) const noexcept {
			return (q != q ||
					l != l ||
					w != w ||
					bl != bh ||
					bl != bl);
		}
	} names;
	bool occupied;
	
	Register(const std::string &q, const std::string &l, const std::string &w, const std::string &bh, const std::string &bl)
		: names({ '%'+q, '%'+l, '%'+w, '%'+bh, '%'+bl }), occupied(false) { }
	Register() : names({"%","%","%","%","%"}), occupied(false) { }

	bool operator==(const Register &reg) const noexcept {
		return (
			names == reg.names &&
			occupied == reg.occupied
		);
	}

	bool operator!=(const Register &reg) const noexcept {
		return (
			names != reg.names ||
			occupied != reg.occupied
		);
	}

	std::string from_size(int size) {
		if (size == 1) {
			return names.bh;
		} else if (size == 2) {
			return names.w;	
		} else if (size == 4) {
			return names.l;
		} else if (size == 8) {
			return names.q;
		}
		return "";
	}

	static bool comp_names(std::string name1, std::string name2) {
		if (name1[0] != '%') name1 = '%' + name1;
		if (name2[0] != '%') name2 = '%' + name2;
		
		return (name1 == name2);
	}
};

using RegisterRef = std::optional<std::reference_wrapper<Register>>; 

enum class BraceType {
	Neutral,
	If
};

struct Brace {
	int brace_index;
	BraceType type;
};

std::vector<std::string> _ltoks;
std::vector<std::string> _us_ltoks;

std::vector<std::string> variable_names = { };
std::vector<int> variable_sizes = { };
std::vector<int> variable_stack_locations = { };

std::vector<Register> registers = {
	Register("rbx","ebx","ax","ah","al"), Register("r10","r10d","r10w","r10b","r10b"), Register("r11","r11d","r11w","r11b","r11b"), Register("r12","r12d","r12w","r12b","r12b"), Register("r13","r13d","r13w","r13b","r13b"), Register("r14","r14d","r13w","r13b","r13b"), Register("r15","r15d","r15w","r15b","r15b"),
	Register("rax","eax","ax","ah","al"), Register("rdi","edi","di","dil","dil"), Register("rsi","esi","si","sil","sil"), Register("rdx","edx","dx","dl","dh"), Register("rcx","ecx","cx","ch","cl"), Register("r8","r8d","r8w","r8b","r8b"), Register("r9","r8d","r8w","r8b","r8b"), // SYSCALL REGISTERS
	Register("rsp","esp","sp","spl","spl"), Register("rbp","ebp","bp","bpl","bpl") // STACK REGISTERS
};

namespace types {
	const std::vector<std::string> types = { "int", "str", "nstr", "lng", "sht", "ch" };
	const std::vector<std::string> asm_types = { ".word", ".asciz", ".ascii", ".long", ".short", ".byte" };
	const std::vector<int> sizes = { 4, 0, 0, 8, 2, 1 };
	const std::vector<std::string> suffixes = { "l", "X", "X", "q", "w", "b" }; // Need to make it a string to bypass weird warnings
};
const std::vector<std::string> math_symbols = { "*", "/", "+", "-", "%" };

std::vector<std::string> out;

using uchar = unsigned char;

const char* const ws = " \t\n\r\f\v";

const std::string SYS_READ  = "$0";
const std::string SYS_WRITE = "$1";
const std::string SYS_EXIT  = "$60";
const std::string STDIN     = "$0";
const std::string STDOUT    = "$1";

size_t line_number = 0;

namespace clog {
	void out(const std::string &str) noexcept {
		std::cout << str << std::endl;
	}
	void warn(const std::string &str, bool print_line = true) noexcept {
		std::cerr << (print_line ? "LINE: " + std::to_string(line_number) : "") << "WARNING: " << str << std::endl;
	}
	void error(const std::string &str, bool print_line = true) noexcept {
		std::cerr << (print_line ? "LINE: " + std::to_string(line_number) : "") << " ERROR: " << str << std::endl;
	}
	void note(const std::string &str) noexcept {
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

template <typename T>
size_t from_it(const std::vector<T> &vec, const typename std::vector<T>::const_iterator &it) {
	return std::distance(vec.begin(), it);
}

template <typename T>
size_t index_of(const std::vector<T> &vec, const T &val) {
	return from_it(vec, std::find(vec.begin(), vec.end(), val));
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

RegisterRef get_available_register() {
	for (Register &reg : registers) {
		if (!reg.occupied) return RegisterRef(reg); 
	}

	return std::nullopt;
}

RegisterRef get_register(std::string str) {
	for (Register &reg : registers) {
		if (reg.comp_names(reg.names.q, str) ||
			reg.comp_names(reg.names.l, str) ||
			reg.comp_names(reg.names.w, str) ||
			reg.comp_names(reg.names.bh, str) ||
			reg.comp_names(reg.names.bl, str)) return RegisterRef(reg);
	}
	
	return std::nullopt;
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

std::string set_operand_prefix(std::string str) {
	if (!get_register(str).has_value() && str.back() != ')') {
		str = '$' + str;
	}

	return str;
}

int get_size_of_asm_stack_variable(const std::string &str) {
	int stack_offset = std::stoi(str.substr(1, str.find('(')-1));
	int var_vec_index = index_of(variable_stack_locations, -stack_offset);

	return variable_sizes[var_vec_index];
}

int get_size_of_register(const std::string &str) {
	for (Register &reg : registers) {
		if (reg.comp_names(reg.names.q, str)) return 8;
		if (reg.comp_names(reg.names.l, str)) return 4;
		if (reg.comp_names(reg.names.w, str)) return 2;
		if (reg.comp_names(reg.names.bh, str)) return 1;
		if (reg.comp_names(reg.names.bl, str)) return 1;
	}

	return -1;
}

int get_size_of_number(const std::string &str) {
	long long number = std::stoll(str);	

	if (number < CHAR_MAX) return 1;
	if (number < SHRT_MAX) return 2;
	if (number < INT_MAX) return 4;
	if (number < LONG_MAX) return 8;
	return 0;
}

int get_size_of_operand(const std::string &str, int number_default = -1) {
	if (str.find("%rbp") != std::string::npos) {
		return get_size_of_asm_stack_variable(str);
	} else if (is_number(str)) {
		if (number_default != -1) return number_default;
		return get_size_of_number(str);
	} else if (RegisterRef reg = get_register(str); reg.has_value()) {
		return get_size_of_register(str);
	} else {
		return -1;
	}
} 

char size_to_letter(int size) {
	if (size == 1)
		return 'b';
	else if (size == 2)
		return 'w';
	else if (size == 4)
		return 'l';
	else if (size == 8)
		return 'q';
	
	return '\0';
}

std::string sign_extension_mov(int lhs, int rhs) {
	return std::string("movs") + size_to_letter(rhs) + size_to_letter(lhs);
}

std::string zero_extension_mov(int lhs, int rhs) {
	return std::string("movz") + size_to_letter(rhs) + size_to_letter(lhs);
}

std::string get_mov_instruction(int lhs, int rhs) {
	if (lhs < rhs)
		return zero_extension_mov(lhs, rhs);
	else
		return "mov" + types::suffixes[index_of(types::sizes, lhs)];
}

namespace token_function {
	void dereference(TokIt tok_it) {
		_us_ltoks.erase(tok_it);
		tok_it->insert(tok_it->begin(), '(');
		tok_it->insert(tok_it->end(), ')');
		commit(_us_ltoks);
	}

	void address_of(TokIt tok_it) {
		RegisterRef reg = get_available_register();
		out.push_back("leaq " + *(tok_it+1) + ", " + reg->get().names.q + '\n');
		
		commit(replace_toks(_us_ltoks, tok_it, tok_it+1, reg->get().names.q));
	}

	void math(TokIt tok_it) {
		std::string cmd;
		if (*tok_it == "*") {
			cmd = "mul";
		} else if (*tok_it == "/") {
			cmd = "div";
		} else if (*tok_it == "+") {
			cmd = "add";
		} else if (*tok_it == "-") {
			cmd = "sub";
		}
		if (cmd == "add" || cmd == "sub") {
			RegisterRef lhs = get_available_register();
			lhs->get().occupied = true;
			RegisterRef rhs = get_available_register();
			
			int lhs_size = get_size_of_operand(*(tok_it-1)); 
			int rhs_size = get_size_of_operand(*(tok_it+1));
			int arithmatic_size = std::max({ lhs_size, rhs_size, 4 });

			// Check again using a number default now that we know the arithmatic size
			lhs_size = get_size_of_operand(*(tok_it-1), arithmatic_size); 
			rhs_size = get_size_of_operand(*(tok_it+1), arithmatic_size);
			
			std::string lhs_str = get_mov_instruction(lhs_size, arithmatic_size) + ' ';
			lhs_str += set_operand_prefix(*(tok_it-1)) +  ", " + lhs->get().from_size(lhs_size)  + '\n';
			std::string rhs_str = get_mov_instruction(rhs_size, arithmatic_size) + ' ';
			rhs_str += set_operand_prefix(*(tok_it+1)) +  ", " + rhs->get().from_size(rhs_size) + '\n';
			
			out.push_back(lhs_str);
			out.push_back(rhs_str);
			out.push_back(cmd + types::suffixes[index_of(types::sizes, arithmatic_size)] + ' ' + rhs->get().from_size(arithmatic_size) + ", " + lhs->get().from_size(arithmatic_size) + '\n');
		
			commit(replace_toks(_us_ltoks, tok_it-1, tok_it+1, lhs->get().from_size(arithmatic_size)));
			
			lhs->get().occupied = true;
			rhs->get().occupied = false;
		} else if (cmd == "mul" || cmd == "div") {
			RegisterRef lhs = get_register("rax");
			lhs->get().occupied = true;
			RegisterRef rhs = get_register("rdx");

			int lhs_size = get_size_of_operand(*(tok_it-1)); 
			int rhs_size = get_size_of_operand(*(tok_it+1));
			int arithmatic_size = std::max({ lhs_size, rhs_size, 4 });

			// Check again using a number default now that we know the arithmatic size
			lhs_size = get_size_of_operand(*(tok_it-1), arithmatic_size); 
			rhs_size = get_size_of_operand(*(tok_it+1), arithmatic_size);
			
			std::string lhs_str = get_mov_instruction(lhs_size, arithmatic_size) + ' ';
			lhs_str += set_operand_prefix(*(tok_it-1)) +  ", " + lhs->get().from_size(lhs_size)  + '\n';
			std::string rhs_str = get_mov_instruction(rhs_size, arithmatic_size) + ' ';
			rhs_str += set_operand_prefix(*(tok_it+1)) +  ", " + rhs->get().from_size(rhs_size) + '\n';
			
			out.push_back(lhs_str);
			out.push_back(rhs_str);
			out.push_back(cmd + types::suffixes[index_of(types::sizes, arithmatic_size)] + ' ' + rhs->get().from_size(arithmatic_size) + '\n');

			commit(replace_toks(_us_ltoks, tok_it-1, tok_it+1, lhs->get().from_size(arithmatic_size)));
			
			lhs->get().occupied = true;
			rhs->get().occupied = false;
		}
	}

	void variable_declaration(TokIt tok_it, std::string current_function, int &current_stack_size) {
		size_t type_vec_index = index_of(types::types, *tok_it);
		
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
			variable_stack_locations.push_back(INT_MAX);
		} else {
			current_stack_size += types::sizes[type_vec_index];
		
			//out.push_back("subq $" + std::to_string(types::sizes[type_vec_index]) + ", %rsp\n");
			
			std::string str = get_mov_instruction(get_size_of_operand(*(tok_it+2)), types::sizes[type_vec_index]) + ' ';
			str += set_operand_prefix(*(tok_it+2)) + ", -" +  std::to_string(current_stack_size) + "(%rbp)\n";
			out.push_back(str);
			variable_names.push_back(*(tok_it+1));
			variable_sizes.push_back(types::sizes[type_vec_index]);
			variable_stack_locations.push_back(-current_stack_size);
		}
	}

	void equals(TokIt tok_it) {
		std::string rhs = *(tok_it+1);
		int rhs_size = get_size_of_operand(rhs);
		size_t type_vec_index = index_of(types::sizes, rhs_size);

		if (RegisterRef rhs_reg = get_register(*(tok_it+1)); rhs_reg.has_value()) {
			rhs_reg->get().occupied = false;
		} else { // "mov mem, mem" is not allowed!!
			RegisterRef reg = get_available_register();
			out.push_back("mov" + types::suffixes[type_vec_index] + ' '  + set_operand_prefix(rhs) + ", " + reg->get().from_size(rhs_size) + '\n');
			rhs = reg->get().from_size(rhs_size);
			reg->get().occupied = false;
		}
		
		out.push_back(get_mov_instruction(rhs_size, get_size_of_operand(*(tok_it-1))) + ' ' + set_operand_prefix(rhs) + ", " + set_operand_prefix(*(tok_it-1)) + '\n');
	}

	void base_functions(TokIt tok_it) {
		if (*(tok_it+1) == "w") { // WRITE
			out.push_back("movq " + SYS_WRITE + ", %rax" + '\n');
			out.push_back("movq " + set_operand_prefix(*(tok_it+2)) + ", %rdi" + '\n');
			out.push_back("movq " + set_operand_prefix(*(tok_it+3)) + ", %rsi" + '\n');
			out.push_back("movq " + set_operand_prefix(*(tok_it+4)) + ", %rdx" + '\n');
			out.push_back("syscall\n");
			
			get_register("rax")->get().occupied = false;
			get_register("rdi")->get().occupied = false;
			get_register("rsi")->get().occupied = false;
			get_register("rdx")->get().occupied = false;
		} else if (*(tok_it+1) == "r") { // READ
			out.push_back("movq " + SYS_READ + ", %rax" + '\n');
			out.push_back("movq " + set_operand_prefix(*(tok_it+2)) + ", %rdi" + '\n');
			out.push_back("movq " + set_operand_prefix(*(tok_it+3)) + ", %rsi" + '\n');
			out.push_back("movq " + set_operand_prefix(*(tok_it+4)) + ", %rdx" + '\n');
			out.push_back("syscall\n");

			get_register("rax")->get().occupied = false;
			get_register("rdi")->get().occupied = false;
			get_register("rsi")->get().occupied = false;
			get_register("rdx")->get().occupied = false;
		} else if (*(tok_it+1) == "e") { // EXIT
			out.push_back("movq " + SYS_EXIT + ", %rax" + '\n');
			out.push_back("movq " + set_operand_prefix(*(tok_it+2)) + ", %rdi" + '\n');
			out.push_back("syscall\n");

			get_register("rax")->get().occupied = false;
			get_register("rdi")->get().occupied = false;
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
		out.push_back("movl $0, %eax\n");
		out.push_back("call " + *tok_it + '\n');
		commit(replace_tok(_us_ltoks, tok_it, "%eax"));
	}
	
	void function_return(TokIt tok_it, const std::vector<std::string> &toks, std::string &current_function) {
		if (tok_it == toks.end()) {
			clog::error(current_function + " is not returning a value. All functions must return a value.", line_number);
			return;
		}

		size_t type_vec_index = index_of(types::sizes, get_size_of_operand(*(tok_it+1)));
		std::string ret = "mov" + types::suffixes[type_vec_index];
		RegisterRef rax = get_register("rax");
		ret += ' ' + set_operand_prefix(*(tok_it+1)) + ", " + rax->get().from_size(types::sizes[type_vec_index]) + '\n';
		out.push_back(ret);
		out.push_back("leave\n");
		out.push_back("ret\n");
		out.push_back(".size " + current_function + ", .-" + current_function + '\n');
		current_function = "";
	}
	
	void brace_begin(TokIt tok_it, std::vector<std::string> toks, std::vector<Brace> &braces, int &braces_index) {
		braces.push_back({ ++braces_index, (index_of(toks, "if") != -1 ? BraceType::If : BraceType::Neutral });
	}

	void brace_end(TokIt tok_it, std::vector<Brace> &braces, int &braces_index, std::vector<std::string> &if_labels) {
		if (braces[braces_index].type == BraceType::If) {
			out.push_back(if_labels); dflkgjsdlfkgj // do this
		}
		braces.erase(braces_index--);
	}

	void if_statement(TokIt tok_it, std::vector<std::string> &if_labels) {
		out.push_back("cmp" + types::suffixes[index_of(types::sizes, get_size_of_operand(*(tok_it+1)))] +
			' ' + *(tok_it+1) + ", " + *(tok_it+3) + '\n'
		);

		std::string op = "";
		if (*(tok_it+2) == "=") {
			op = "ne";
		} else if (*(tok_it+2) == "!=") {
			op = "e";
		} else if (*(tok_it+2) == "<") {
			op = "ge";
		} else if (*(tok_it+2) == "<=") {
			op = "g";
		} else if (*(tok_it+2) == ">") {
			op = "le";
		} else if (*(tok_it+2)  == ">=") {
			op = "l";
		}

		out.push_back('j' + op + "IF" + std::to_string(if_labels.size()) + '\n');
		if_labels.push_back("IF" + std::to_string(if_labels.size()));
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

	std::vector<std::string> functions = { };
	std::vector<int> current_function_stack_sizes = { };
	std::string current_function = "";
	
	std::vector<Brace> open_braces = { };
	int brace_index = 0u;

	std::vector<std::string> if_labels = { };

	// --------- MAIN ---------
	out.push_back(".text\n");
	// ------------------------
	
	registers.push_back(Register());
	
	TokIt tok_it;

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
			WHILE_US_FIND_TOKENS(variable_names) {
				size_t vec_index = index_of(variable_names, *tok_it);
				if (variable_stack_locations[vec_index] != INT_MAX) {
					commit(replace_tok(_us_ltoks, tok_it, std::to_string(variable_stack_locations[vec_index]) + "(%rbp)"));
				}
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKENS(math_symbols) {
				token_function::math(tok_it);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKENS(types::types) {
				size_t func_vec_index = from_it(functions, std::find(functions.begin(), functions.end(), current_function));
				token_function::variable_declaration(tok_it, current_function, current_function_stack_sizes[func_vec_index]);
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
			WHILE_US_FIND_TOKEN("=") {
				token_function::equals(tok_it);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN(">") {
				token_function::base_functions(tok_it);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("if") {
				token_function::if_statement(tok_it, if_labels);
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

	system(((std::string)"as rcout.s -o rcout.o && ld rcout.o -e main && ./a.out").c_str());
	system(((std::string)"rm rcout.o").c_str());

	return 0;
}

