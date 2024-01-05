#include <algorithm>
#include <functional>
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <climits>
#include <optional>
#include <ranges>
#include "Token.h"

#define WHILE_FIND_TOKEN(tok) \
	tok_it = _ltoks.begin(); \
	while (true) { \
		tok_it = remove_constness(_ltoks, find_tok(_ltoks, tok, tok_it)); \
		if (tok_it != _ltoks.end()) { \
			if (print_line_in_asm) out.push_back("//" + std::string(tok) + " on line " + std::to_string(line_number) + '\n');
 
#define WHILE_FIND_TOKEN_END \
		} else { \
			break; \
		} \
		tok_it++; \
	} \

#define WHILE_US_FIND_TOKEN(tok) \
	tok_it = _us_ltoks.begin(); \
	while (true) { \
		tok_it = remove_constness(_us_ltoks, find_tok(_us_ltoks, tok, tok_it)); \
		if (tok_it != _us_ltoks.end()) { \
			if (print_line_in_asm) out.push_back("//" + std::string(tok) + " on line " + std::to_string(line_number) + '\n');

#define WHILE_FIND_TOKENS(toks) \
	tok_it = _ltoks.begin(); \
	while (true) { \
		tok_it = remove_constness(_ltoks, find_first_tok(_ltoks, toks, tok_it)); \
		if (tok_it != _ltoks.end()) { \
			if (print_line_in_asm) { \
				std::string str = "// "; \
				for (const std::string &tok : toks) { \
					str += tok + " | "; \
				} \
				str += "on line " + std::to_string(line_number); \
				out.push_back(str + '\n'); \
			}

#define WHILE_US_FIND_TOKENS(toks) \
	tok_it = _us_ltoks.begin(); \
	while (true) { \
		tok_it = remove_constness(_us_ltoks, find_first_tok(_us_ltoks, toks, tok_it)); \
		if (tok_it != _us_ltoks.end()) { \
			if (print_line_in_asm) { \
				std::string str = "// "; \
				for (const std::string &tok : toks) { \
					str += tok + " | "; \
				} \
				str += "on line " + std::to_string(line_number); \
				out.push_back(str + '\n'); \
			}

#define DATA_ASM (std::ranges::find(out, ".data\n"))
#define TEXT_ASM (std::ranges::find(out, ".text\n"))

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

	std::string name_from_size(int size) {
		if (size == 1) {
			return names.bl;
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

std::vector<std::string> _ltoks;
std::vector<std::string> _us_ltoks;

std::vector<std::string> variable_names = { };
std::vector<int> variable_sizes = { };
std::vector<int> variable_stack_locations = { };
std::vector<bool> variable_is_pointer = { };
std::vector<int> variable_true_pointer_size = { };

std::vector<std::pair<std::string, std::string>> dereferenced_register_variable_correspondant = { };

std::vector<Register> registers = {
	Register("rbx","ebx","bx","bh","bl"), Register("r10","r10d","r10w","r10b","r10b"), Register("r11","r11d","r11w","r11b","r11b"), Register("r12","r12d","r12w","r12b","r12b"), Register("r13","r13d","r13w","r13b","r13b"), Register("r14","r14d","r13w","r13b","r13b"), Register("r15","r15d","r15w","r15b","r15b"),
	Register("rax","eax","ax","ah","al"), Register("rdi","edi","di","dil","dil"), Register("rsi","esi","si","sil","sil"), Register("rdx","edx","dx","dl","dh"), Register("rcx","ecx","cx","ch","cl"), Register("r8","r8d","r8w","r8b","r8b"), Register("r9","r8d","r8w","r8b","r8b"), // SYSCALL REGISTERS
	Register("rsp","esp","sp","spl","spl"), Register("rbp","ebp","bp","bpl","bpl") // STACK REGISTERS
};

namespace types {
	const std::vector<std::string> types = { "int", "lng", "sht", "ch" };
	const std::vector<std::string> asm_types = { ".word", ".long", ".short", ".byte" };
	const std::vector<int> sizes = { 4, 8, 2, 1 };
	const std::vector<std::string> suffixes = { "l", "q", "w", "b" }; // Need to make it a string to bypass weird warnings
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

template <typename T>
size_t from_it(const std::vector<T> &vec, const typename std::vector<T>::const_iterator &it) {
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
	return replace_tok(toks, from_it(toks, it), str);
}

std::string combine_toks(const std::vector<std::string>::const_iterator &begin, const std::vector<std::string>::const_iterator &end) {
	std::string ret = "";
	for (auto it = begin; it != end; it++) {
		ret += *it;
	}
	return ret;
}

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
	
	size_t quote_begin = 0;
	bool in_quotes = false;
	for (int i = 0; i < ret.size(); i++) {
		if (ret[i] == "\"") {
			if (!in_quotes) {
				quote_begin = i;
				in_quotes = true;
				continue;
	  		} else {
				in_quotes = false;
				ret = replace_toks(ret, ret.begin()+quote_begin+1, ret.begin()+i-1, combine_toks(ret.begin()+quote_begin+1, ret.begin()+i));
			}
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
	
	for (auto it = begin; it != ltoks.end(); it++) {
		for (const std::string &tok : toks_to_find) {
			if (*it == tok) return it;
		}
	}

	return ltoks.end();
}


template <typename T>
size_t index_of(const std::vector<T> &vec, const T &val) {
	return from_it(vec, std::find(vec.begin(), vec.end(), val));
}

template <typename T>
size_t index_of_last(const std::vector<T> &vec, const T &val) {
	return from_it(vec, std::find(vec.rbegin(), vec.rend(), val).base());
}


void commit(const std::vector<std::string> &new_vec) {
	_ltoks = new_vec;
	if (find_tok(new_vec, " ", new_vec.begin()) ==	new_vec.end()) {
		_us_ltoks = new_vec;
	} else {
		_us_ltoks = unspaced(_ltoks);
	}
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
    while (it != s.end() && (std::isdigit(*it) || *it == '-')) ++it;
    return !s.empty() && it == s.end();
}

bool is_stack_variable(const std::string &str) {
	// ex of variable: -4(%rbp)
	if (str.find("%rbp") != std::string::npos) {
		size_t parenthesis = str.find('(');
		if (parenthesis != -1) {
			std::string num_before = str.substr(0, parenthesis);
			if (is_number(num_before)) {
				if (std::stoi(num_before) < 0) {
					std::string reg = str.substr(parenthesis, str.size()-parenthesis);
					if (reg == "(%rbp)") {
						return true;
					}
				}
			}
		}
	}
	
	return false;
}

bool is_global_variable(const std::string &str) {
	if (str.find("%rip") != std::string::npos) {
		return true;
	}
	return false;
}

bool is_variable(const std::string &str) {
	return (is_stack_variable(str) || is_global_variable(str));
}

bool is_dereferenced(const std::string &str) {
	return (str.front() == '(' && str.back() == ')');
}

//bool is_pointer(const std::string &str) {
//	return str.find("(%rip)") != std::string::npos;
//}

std::string set_operand_prefix(std::string str) {
	if (!get_register(str).has_value() && str.back() != ')') {
		str = '$' + str;
	}

	return str;
}

int get_size_of_asm_variable(const std::string &str) {
	if (str.find("%rbp") != std::string::npos) {
		int stack_offset = std::stoi(str.substr(1, str.find('(')-1));
		int var_vec_index = index_of(variable_stack_locations, -stack_offset);
		
		return variable_sizes[var_vec_index];
	} else if (str.find("%rip") != std::string::npos) {
		return variable_sizes[index_of(variable_names, str.substr(0, str.find('(')))];
	}

	return -1;
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

int get_size_of_operand(std::string str, int number_default = -1) {
	int variable_index = index_of(variable_names, str);
	if (is_dereferenced(str)) {
		str.erase(str.begin());
		str.pop_back();
		if (RegisterRef reg = get_register(str); reg.has_value()) {
			auto corresponding_variable_it = std::ranges::find_if(dereferenced_register_variable_correspondant, [&](auto p){ return (p.first == str); });
			variable_index = index_of(variable_names, corresponding_variable_it->second);
		}
		return variable_true_pointer_size[variable_index];
	} if (is_variable(str)) {
		return get_size_of_asm_variable(str);
	} else if (is_number(str)) {
		if (number_default != -1) return number_default;
		return get_size_of_number(str);
	} else if (RegisterRef reg = get_register(str); reg.has_value()) {
		return get_size_of_register(str);
	} else if (auto it = std::ranges::find(variable_names, str); it != variable_names.end()) {
		return variable_sizes[variable_index];	
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
	return std::string("movs") + size_to_letter(lhs) + size_to_letter(rhs);
}

std::string zero_extension_mov(int lhs, int rhs) {
	return std::string("movz") + size_to_letter(lhs) + size_to_letter(rhs);
}

std::string get_mov_instruction(int lhs, int rhs) {
	if (lhs < rhs)
		return sign_extension_mov(lhs, rhs);
	else
		return "mov" + (rhs != -1 ? types::suffixes[index_of(types::sizes, rhs)] : "q");
}

std::string get_mov_instruction(const std::string &lhs, const std::string &rhs) {
	int rhs_size = get_size_of_operand(rhs);
	int lhs_size;
	//if (is_dereferenced(rhs)) {
	//	lhs_size = get_size_of_operand(lhs);
	//	rhs_size = lhs_size;
	//} else {
		lhs_size = get_size_of_operand(lhs, rhs_size);
	//}
	
	//return get_mov_instruction((is_pointer(lhs) ? rhs_size : lhs_size), rhs_size);
	return get_mov_instruction(/*(lhs.find("(%rip)") != std::string::npos ? rhs_size : lhs_size)*/lhs_size, rhs_size);
}

std::string mov(const std::string &lhs, const std::string &rhs) {
	return get_mov_instruction(lhs, rhs) + ' ' + set_operand_prefix(lhs) + ", " + set_operand_prefix(rhs);
}

void unoccupy_if_register(const std::string &reg_name) {
	if (RegisterRef reg = get_register(reg_name); reg.has_value()) {
		reg->get().occupied = false;
	}
}

std::pair<std::string, std::string> cast_lhs_rhs(std::string lhs, std::string rhs, int default_size = -1, bool change_reg_size = true) {
	int rhs_size = get_size_of_operand(rhs);
	int lhs_size = get_size_of_operand(lhs, rhs_size);
	int max_size;
	if (default_size == -1) {
		max_size = std::max(lhs_size, rhs_size);
	} else {
		max_size = std::max({ lhs_size, rhs_size, default_size });
	}
	
	bool lhs_is_reg = true;
	if (!get_register(lhs).has_value()) {
		lhs_is_reg = false;
	}
	bool rhs_is_reg = true;
	if (!get_register(rhs).has_value()) {
		rhs_is_reg = false;
	}
	
	bool lhs_is_number = is_number(lhs);
	bool rhs_is_number = is_number(rhs);
	
	// mem -> mem is not allowed, so I have to make the lhs a register.
	// This would also be a good time to cast the lhs to the appropriate size.
	if (!(rhs_is_reg || rhs_is_number) && !(lhs_is_reg || lhs_is_number)) { // if both are memory
		unoccupy_if_register(lhs);
		//if (is_pointer(lhs) && is_pointer(rhs)) {
		//	RegisterRef reg = get_available_register();
		//	reg->get().occupied = true;
		//	out.push_back("movq " + lhs + ", " + reg->get().name_from_size(8) + '\n');
		//	lhs = reg->get().name_from_size(8);
		//} else if (is_pointer(lhs) && !is_pointer(rhs)) {
		//	
		//} else
		if (lhs_size == 4 && max_size == 8) { // int -> long
			out.push_back("movl " + set_operand_prefix(lhs) + ", %eax\n");
			out.push_back("cltq\n");
			
			lhs = "%rax"; // Using 'rax' is probably a problem. Do I care right now though? No.
		} else if (lhs_size == 1 && max_size == 4) { // byte -> int
			RegisterRef reg = get_available_register();
			out.push_back("movsbl " + set_operand_prefix(lhs) + ", " + reg->get().name_from_size(4) + '\n');
			lhs = reg->get().name_from_size(4);
		} else {
			RegisterRef reg = get_available_register();
			reg->get().occupied = true;
			out.push_back(get_mov_instruction(lhs_size, max_size) + ' ' + set_operand_prefix(lhs) + ", " + reg->get().name_from_size(max_size) + '\n');
			lhs = reg->get().name_from_size(max_size);
		}
	}
	
	if (change_reg_size) {
		if (RegisterRef reg = get_register(lhs); reg.has_value()) {
			reg->get().occupied = true;
			lhs = reg->get().name_from_size(rhs_size);
		}
	}

	return { lhs, rhs };
}

namespace token_function {
	void dereference(TokIt &tok_it) {
		_us_ltoks.erase(tok_it);
		std::string var_name = "";
		std::string before_parenthesis = tok_it->substr(0, tok_it->find('('));
		if (is_stack_variable(*tok_it)) {
			var_name = variable_names[index_of(variable_stack_locations, std::stoi(before_parenthesis))];
		} else if (is_global_variable(*tok_it)) {
			var_name = before_parenthesis;
		} else {
			clog::error("Dereference: Only pointers can be dereferenced");
		}

		RegisterRef reg = get_available_register();
		reg->get().occupied = true;
		std::string reg_name = reg->get().name_from_size(8);
		out.push_back("movq " + *(tok_it) + ", " + reg_name + '\n');
		commit(replace_tok(_us_ltoks, tok_it, reg_name));
		commit(replace_tok(_us_ltoks, tok_it, '(' + *tok_it + ')'));
		dereferenced_register_variable_correspondant.push_back(std::make_pair(reg_name, var_name));
	}

	void address_of(TokIt tok_it) {
		RegisterRef reg = get_available_register();
		out.push_back("leaq " + *(tok_it+1) + ", " + reg->get().names.q + '\n');
		
		commit(replace_toks(_us_ltoks, tok_it, tok_it+1, reg->get().names.q));
	}

	void math(TokIt &tok_it) {
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
			int rhs_size = get_size_of_operand(*(tok_it-1));
			RegisterRef rhs = get_available_register(); // asm rhs (math output)
			rhs->get().occupied = true;
			std::string rhs_name =  rhs->get().name_from_size(rhs_size);
			
			// I don't remember why I wrote this. That's how you know my code
			// is well documented and easy to understand. I wrote this last week and already forgot!!
			
			/*std::string lhs = *(tok_it-1); // asm lhs (math input)
			RegisterRef register_ref;
			if (!is_variable(lhs)) {
				register_ref = get_available_register();
				register_ref->get().occupied = true;
				std::string reg = register_ref->get().name_from_size(rhs_size);
				out.push_back(get_mov_instruction(rhs_size, rhs_size) + ' ' + set_operand_prefix(lhs) + ", " + reg + '\n');
			}
			*/

			// 'rhs' will store result.
			out.push_back(mov(*(tok_it-1), rhs_name) + '\n');
			
			std::pair<std::string, std::string> lhs_rhs = cast_lhs_rhs(*(tok_it+1), rhs_name, 4);
			out.push_back(
				cmd + types::suffixes[index_of(types::sizes, get_size_of_operand(lhs_rhs.second))] + ' ' +
				set_operand_prefix(lhs_rhs.first) + ", " + set_operand_prefix(lhs_rhs.second) + '\n'
			);
			commit(replace_toks(_us_ltoks, tok_it-1, tok_it+1, lhs_rhs.second));
			tok_it--; // Tok it is out of bounds since "[x] [+] [y]" narrows down to "[result]"
			
			rhs->get().occupied = false;
			if (RegisterRef reg = get_register(lhs_rhs.second); reg.has_value()) {
				reg->get().occupied = true;
			}
			unoccupy_if_register(lhs_rhs.first);
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
			lhs_str += set_operand_prefix(*(tok_it-1)) +  ", " + lhs->get().name_from_size(lhs_size)  + '\n';
			std::string rhs_str = get_mov_instruction(rhs_size, arithmatic_size) + ' ';
			rhs_str += set_operand_prefix(*(tok_it+1)) +  ", " + rhs->get().name_from_size(rhs_size) + '\n';
			
			out.push_back(lhs_str);
			out.push_back(rhs_str);
			out.push_back(cmd + types::suffixes[index_of(types::sizes, arithmatic_size)] + ' ' + rhs->get().name_from_size(arithmatic_size) + '\n');

			commit(replace_toks(_us_ltoks, tok_it-1, tok_it+1, lhs->get().name_from_size(arithmatic_size)));

			lhs->get().occupied = true;
			rhs->get().occupied = false;
		}
	}

	void variable_declaration(const std::string &type, const std::string &name, std::string value,
						   const std::string &current_function, int &current_stack_size, bool is_pointer) {
		size_t type_vec_index = index_of(types::types, type);
		
		if (DATA_ASM == out.end()) {
			out.insert(TEXT_ASM, ".data\n");
		}
		
		if (is_pointer) {
			variable_declaration("lng", name, value, current_function, current_stack_size, false);
			variable_is_pointer.back() = true;
			variable_true_pointer_size.back() = types::sizes[type_vec_index];
		} else if (current_function.empty()) {
			out.insert(DATA_ASM+1, ".globl " + name + '\n');
			out.insert(DATA_ASM+2, ".align 8\n");
			out.insert(DATA_ASM+3, ".type " + name + ", @object\n");
			out.insert(DATA_ASM+4, ".size " + name + ", " + std::to_string(types::sizes[type_vec_index]) + '\n');
			out.insert(DATA_ASM+5, name + ":\n");
			
			out.insert(DATA_ASM+6, types::asm_types[type_vec_index] + ' ' + value + '\n');
			
			variable_names.push_back(name);
			variable_sizes.push_back(types::sizes[type_vec_index]);
			variable_stack_locations.push_back(INT_MAX);
			variable_is_pointer.push_back(false);
			variable_true_pointer_size.push_back(-1);
		} else {
			current_stack_size += types::sizes[type_vec_index];
			variable_names.push_back(name);
			variable_sizes.push_back(types::sizes[type_vec_index]);
			variable_stack_locations.push_back(-current_stack_size);
			variable_is_pointer.push_back(false);
			variable_true_pointer_size.push_back(-1);
			std::pair<std::string, std::string> lhs_rhs = cast_lhs_rhs(value, name);
			std::string str = get_mov_instruction(lhs_rhs.first, lhs_rhs.second) + ' ';
			str += set_operand_prefix(lhs_rhs.first) + ", -" +  std::to_string(current_stack_size) + "(%rbp)\n";
			out.push_back(str);
			unoccupy_if_register(lhs_rhs.first);
			unoccupy_if_register(lhs_rhs.second);
		}
	}

	void variable_declaration(TokIt tok_it, const std::string &current_function, int &current_stack_size, bool is_pointer) {
		std::string value = "0";
		if (tok_it+2 != _us_ltoks.end()) {
			value = *(tok_it+2);
		}
		variable_declaration(*tok_it, *(tok_it+1), value, current_function, current_stack_size, is_pointer);
	}
	
	void equals(const std::string &lhs, const std::string &rhs, bool change_reg_size = false) {
		std::pair<std::string, std::string> lhs_rhs = cast_lhs_rhs(lhs, rhs, get_size_of_operand(rhs), change_reg_size);
		
		out.push_back(mov(lhs_rhs.first, lhs_rhs.second) + '\n');
		
		unoccupy_if_register(lhs_rhs.first);
		unoccupy_if_register(lhs_rhs.second);
	}
	
	void equals(TokIt tok_it) {
		equals(*(tok_it+1), *(tok_it-1), true);
	}

	void base_functions(TokIt tok_it) {
		if (*(tok_it+1) == "w") { // WRITE
			out.push_back("movq " + SYS_WRITE + ", %rax" + '\n');
			//equals(*(tok_it+2), get_register("%rdi")->get().name_from_size(get_size_of_operand(*(tok_it+2))));
			//equals(*(tok_it+3), get_register("%rsi")->get().name_from_size(get_size_of_operand(*(tok_it+3))));
			//equals(*(tok_it+4), get_register("%rdx")->get().name_from_size(get_size_of_operand(*(tok_it+4))));
			equals(*(tok_it+2), "%rdi");
			equals(*(tok_it+3), "%rsi");
			equals(*(tok_it+4), "%rdx");
			out.push_back("syscall\n");
			unoccupy_if_register(*(tok_it+2));
			unoccupy_if_register(*(tok_it+3));
			unoccupy_if_register(*(tok_it+4));
		} else if (*(tok_it+1) == "r") { // READ
			out.push_back("movq " + SYS_READ + ", %rax" + '\n');
			//equals(*(tok_it+2), get_register("%rdi")->get().name_from_size(get_size_of_operand(*(tok_it+2))));
			//equals(*(tok_it+3), get_register("%rsi")->get().name_from_size(get_size_of_operand(*(tok_it+3))));
			//equals(*(tok_it+4), get_register("%rdx")->get().name_from_size(get_size_of_operand(*(tok_it+4))));
			equals(*(tok_it+2), "%rdi");
			equals(*(tok_it+3), "%rsi");
			equals(*(tok_it+4), "%rdx");
			out.push_back("syscall\n");
			unoccupy_if_register(*(tok_it+2));
			unoccupy_if_register(*(tok_it+3));
			unoccupy_if_register(*(tok_it+4));
		} else if (*(tok_it+1) == "e") { // EXIT
			out.push_back("movq " + SYS_EXIT + ", %rax" + '\n');
			//equals(*(tok_it+2), get_register("%rdi")->get().name_from_size(get_size_of_operand(*(tok_it+2))));
			equals(*(tok_it+2), "%rdi");
			out.push_back("syscall\n");
			unoccupy_if_register(*(tok_it+2));
		}
	}

	void function_declaration(TokIt tok_it, std::vector<std::string> &functions, std::vector<int> &stack_sizes, std::string &current_function) {
		if (std::ranges::find(out, ".text\n") == out.end()) {
			out.push_back(".text\n");
		}
		
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
		if (tok_it+1 == toks.end()) {
			clog::error(current_function + " is not returning a value. All functions must return a value.", line_number);
			return;
		}

		size_t type_vec_index = index_of(types::sizes, get_size_of_operand(*(tok_it+1)));
		std::string ret = "mov" + types::suffixes[type_vec_index];
		RegisterRef rax = get_register("rax");
		ret += ' ' + set_operand_prefix(*(tok_it+1)) + ", " + rax->get().name_from_size(types::sizes[type_vec_index]) + '\n';
		out.push_back(ret);
		out.push_back("leave\n");
		out.push_back("ret\n");
		out.push_back(".size " + current_function + ", .-" + current_function + '\n');
		current_function = "";
	}

	void if_statement(TokIt tok_it, int &if_index) {
		std::string rhs = *(tok_it-3);
		std::string lhs = *(tok_it-1);
		
		std::function<void(std::string&)> change_to_reg = [](std::string &str) {
			RegisterRef reg = get_available_register();
			reg->get().occupied = true;
			out.push_back(mov(str, reg->get().name_from_size(get_size_of_number(str))) + '\n');
			str = reg->get().name_from_size(get_size_of_number(str));
		}; // Trying to somewhat stay DRY...
		
		if (is_number(rhs)) {
			change_to_reg(rhs);
		}
		if (is_number(lhs)) {
			change_to_reg(lhs);
		}
		
		std::pair<std::string, std::string> lhs_rhs = cast_lhs_rhs(lhs, rhs);
		
		// Insert before call instruction
		out.push_back("cmp" + types::suffixes[index_of(types::sizes, get_size_of_operand(lhs_rhs.second))] +
			' ' + set_operand_prefix(lhs_rhs.first) + ", " + set_operand_prefix(lhs_rhs.second) + '\n'
		);

		std::string op = "";
		if (*(tok_it-2) == "==") {
			op = "ne";
		} else if (*(tok_it-2) == "!=") {
			op = "e";
		} else if (*(tok_it-2) == "<") {
			op = "ge";
		} else if (*(tok_it-2) == "<=") {
			op = "g";
		} else if (*(tok_it-2) == ">") {
			op = "le";
		} else if (*(tok_it-2)  == ">=") {
			op = "l";
		}
		
		out.push_back('j' + op + " .IF" + std::to_string(if_index) + '\n');
		if_index++;
	}

	void if_end(TokIt tok_it, int if_index) {
		out.push_back(".IF" + std::to_string(if_index-1) + ":\n");
	}
	
	void else_statement(TokIt tok_it, int &if_index) {
		out.insert(out.end()-1, "jmp .IF" + std::to_string(if_index) + '\n');
		if_index++;
	}
	
	void else_end(TokIt tok_it, int if_index) {
		out.push_back(".IF" + std::to_string(if_index-1) + ":\n");	
	}
	
	void macro(TokIt tok_it, std::vector<std::string> &lines) {
		if (*(tok_it+1) == "inc") {
			/*std::ifstream in(*(tok_it+2));
			
			std::vector<std::string> inc_lines = { };
			std::string l;
			while (std::getline(in, l)) {
				inc_lines.push_back(l);
			}
			
			std::cout << *(tok_it+2) << std::endl;
			lines.insert(lines.begin()+line_number+1, inc_lines.begin(), inc_lines.end());*/
			out.push_back(".include " + combine_toks(tok_it+2, _us_ltoks.end()) + '\n');
		}
	}
	
	void quote(TokIt tok_it, int &str_index, bool &in_quote) {
		if (DATA_ASM == out.end()) {
			out.insert(TEXT_ASM, ".data\n");
		}
		if (in_quote) {
			out.insert(DATA_ASM+1, ".STR" + std::to_string(str_index) + ":\n");
			
			std::string str;
			if (tok_it != _us_ltoks.begin()) {
				str = (*(tok_it-3) == "n" ? ".ascii" : ".asciz");
			} else {
				str = ".asciz";
			}
			
			out.insert(DATA_ASM+2, str + ' ' + combine_toks(tok_it-2, tok_it+1) + std::string(1, '\n'));
			commit(replace_toks(_us_ltoks, (str == ".ascii" ? tok_it-3 : tok_it-2), tok_it, ".STR" + std::to_string(str_index)));
			variable_names.push_back(".STR" + std::to_string(str_index));
			variable_sizes.push_back(8);
			variable_stack_locations.push_back(INT_MAX);
			str_index++;
		}
		in_quote = !in_quote;
	}
}

int begin_compile(std::vector<std::string> args) {
	std::string output_dir = "./";
	bool print_line_in_asm = false;
	if (args.size() < 2) {
		clog::error("Must input file to compile.", 0);
		return 0;
	} else if (args.size() == 3) {
		if (args[2] == "true") {
			print_line_in_asm = true;
		}
	} else if (auto it = std::ranges::find(args, "-od"); it != args.end()) {
		output_dir = *(it+1);
		if ((it+1)->back() != '/') output_dir += '/';
	}

	std::ifstream read;
	read.open(args[1], std::ifstream::in);
	std::ofstream write;
	write.open(output_dir + "rcout.s", std::ofstream::out | std::ios::trunc);
	
	std::vector<std::string> disallowed_toks = { };

	std::vector<std::string> functions = { };
	std::vector<int> current_function_stack_sizes = { };
	std::string current_function = "";
	
	int if_index = 0;
	int str_index = 0;
	bool in_quote = false;

	registers.push_back(Register());
	
	TokIt tok_it;
	
	std::vector<std::string> lines = { };
	{
		std::string l;	
		while (std::getline(read, l)) {
			lines.push_back(l);
		}
	}

	for (std::string l : lines) {
		line_number++;
		l = trim(l);
		if (!l.empty()) {
			_ltoks = split(l);
			_us_ltoks = unspaced(_ltoks);
			
			//std::ranges::for_each(_us_ltoks,[](auto s){std::cout<<s<<' ';});std::cout<<std::endl; // print all toks
			
			WHILE_US_FIND_TOKEN("//") {
				int i = std::distance(_us_ltoks.begin(), tok_it);
				while ( i < _us_ltoks.size()) {
					_us_ltoks.erase(_us_ltoks.begin()+i);
				}
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("\"") {
				token_function::quote(tok_it, str_index, in_quote);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("#") {
				token_function::function_declaration(tok_it, functions, current_function_stack_sizes, current_function);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKENS(functions) {
				if (std::distance(_us_ltoks.begin(), tok_it) > 0) {
					if (*(tok_it-1) == "#") {
						break;
					}
				}
				token_function::function_call(tok_it);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKENS(variable_names) {
				size_t vec_index = index_of(variable_names, *tok_it);
				if (variable_stack_locations[vec_index] != INT_MAX) {
					commit(replace_tok(_us_ltoks, tok_it, std::to_string(variable_stack_locations[vec_index]) + "(%rbp)"));
				} else {
					*tok_it += "(%rip)";
					commit(_us_ltoks);
				}
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("^") {
				token_function::dereference(tok_it);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("&") {
				token_function::address_of(tok_it);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKENS(math_symbols) {
				token_function::math(tok_it);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("=") {
				token_function::equals(tok_it);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKENS(types::types) {
				size_t func_vec_index = from_it(functions, std::ranges::find(functions, current_function));
				bool is_pointer = false;
				if (tok_it != _us_ltoks.begin()) {
					is_pointer = (*(tok_it-1) == "^^");
				}
				//if (functions.empty() || current_function.empty()) {
				//	int ss;
				//	token_function::variable_declaration(tok_it, "", ss, is_pointer);
				//} else {
					token_function::variable_declaration(tok_it, current_function, current_function_stack_sizes[func_vec_index], is_pointer);
				//}
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("#>") {
				token_function::function_return(tok_it, _us_ltoks, current_function);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("?>") {
				token_function::if_end(tok_it, if_index);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("??>") {
				token_function::else_end(tok_it, if_index);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("?") {
				token_function::if_statement(tok_it, if_index);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("??") {
				token_function::else_statement(tok_it, if_index);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("~") {
				commit(replace_tok(_us_ltoks, tok_it, "rax"));
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN(">") {
				token_function::base_functions(tok_it);
			} WHILE_FIND_TOKEN_END
			WHILE_US_FIND_TOKEN("%") {
				token_function::macro(tok_it, lines);
			} WHILE_FIND_TOKEN_END

			disallowed_toks.clear();

			for (Register &reg : registers) {
				reg.occupied = false;
			}
		}
	}

	out.insert(out.begin(), ".file \"" + std::string(args[1]) + "\"\n");

	for (const std::string &str : out) {
		write << str;
	}
	
	read.close();
	write.close();
	
	system(((std::string)"as " + output_dir + "rcout.s -o " + output_dir + "rcout.o && ld " + output_dir + "rcout.o -e main -o " + output_dir + "a.out && " + output_dir + "a.out").c_str());
	
	return 0;
}

