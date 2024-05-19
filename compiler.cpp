#include <algorithm>
#include <functional>
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <climits>
#include <optional>
#include <ranges>
#include <utility>
#include "preprocessor.cpp" // Including in here stops weird redefinition errors for util.h

#define DATA_ASM (std::ranges::find(out, ".data\n"))
#define TEXT_ASM (std::ranges::find(out, ".text\n"))

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

struct Brace {
	enum class State {
		Open,
		Close
	};

	enum class Type {
		If,
		Else,
		Function,
		General,
		While,
	};
	
	Brace() { }
	Brace(State state, Type type, int index, int type_index) : state(state), type(type), type_index(type_index), index(index) { }

	State state;
	Type type;
	int type_index = -1;
	int index = 0;
};

using BraceRef = std::optional<std::reference_wrapper<Brace>>;

namespace braces {
	std::vector<Brace> braces = { Brace(Brace::State::Open, Brace::Type::Function, -1, -1) };
	
	Brace & get_last(std::vector<Brace::Type> types, bool open = false) {
		for (Brace &brace : braces | std::views::reverse) {
			bool eq = false;
			for (Brace::Type type : types) { eq = (brace.type == type); if (eq) break; }
			if (open) eq = brace.state == Brace::State::Open;
			if (eq) {
				return brace;
			}
		}
		return braces.front();
	}
	
	Brace & get_last_condition(bool open = false) {
		return get_last({ Brace::Type::If, Brace::Type::Else, Brace::Type::While }, open);
	}
	
	Brace & get_last_index(int index) {
		for (Brace &brace : braces | std::views::reverse) {
			if (brace.index == index) {
				return brace;
			}
		}
		return braces.front();
	}
	
	Brace & get_last_open_index(int index) {
		for (Brace &brace : braces | std::views::reverse) {
			if (brace.state == Brace::State::Open && brace.index == index) {
				return brace;
			}
		}
		return braces.front();
	}
	
	int braces_begin_index(bool type = false) {
		if (braces::braces.back().state == Brace::State::Close) {
			return braces::braces.back().index;
		} else {
			return braces::braces.back().index+1;
		}
	}

	int braces_end_index(bool type = false) {
		int index = 0;
		if (braces::braces.back().state == Brace::State::Open) {
			index = braces::braces.back().index;
		} else {
			index = braces::get_last_open_index(braces::braces.back().index-1).index;
		}
		Brace last_index = braces::get_last_index(index);
		if (type) {
			return last_index.type_index;
		} else {
			return last_index.index;
		}
	}
};

struct Variable {
	std::string name;
	int size;
	int stack_location;
	bool is_pointer;
	int true_pointer_size;
	int braces_index;
};

std::vector<Variable> variables;

std::vector<std::string> variable_names() {
	std::vector<std::string> names{};
	for (Variable &var : variables) {
		names.push_back(var.name);
	}
	return names;
}

std::vector<int> variable_sizes() {
	std::vector<int> sizes{};
	for (Variable &var : variables) {
		sizes.push_back(var.size);
	}
	return sizes;
}

std::vector<int> variable_stack_locations() {
	std::vector<int> stack_locations{};
	for (Variable &var : variables) {
		stack_locations.push_back(var.stack_location);
	}
	return stack_locations;
}

std::vector<bool> variable_is_pointer() {
	std::vector<bool> is_pointers{};
	for (Variable &var : variables) {
		is_pointers.push_back(var.is_pointer);
	}
	return is_pointers;
}

std::vector<int> variable_true_pointer_size() {
	std::vector<int> true_pointer_sizes{};
	for (Variable &var : variables) {
		true_pointer_sizes.push_back(var.true_pointer_size);
	}
	return true_pointer_sizes;
}

std::vector<int> variable_braces_index() {
	std::vector<int> braces_indices{};
	for (Variable &var : variables) {
		braces_indices.push_back(var.braces_index);
	}
	return braces_indices;
}

std::vector<std::pair<std::string, std::string>> dereferenced_register_variable_correspondant = { };

std::string current_function = "";

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

const std::vector<std::string> math_symbols = { "*", "/", "+", "-" };

std::vector<std::string> out;

const std::string SYS_READ  = "$0";
const std::string SYS_WRITE = "$1";
const std::string SYS_EXIT  = "$60";
const std::string STDIN     = "$0";
const std::string STDOUT    = "$1";

RegisterRef get_available_register() {
	for (Register &reg : registers) {
		if (!reg.occupied) return RegisterRef(reg); 
	}

	clog::error("No registers are available.");
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
		int var_vec_index = index_of(variable_stack_locations(), -stack_offset);
		
		return variables[var_vec_index].size;
	} else if (str.find("%rip") != std::string::npos) {
		return variables[index_of(variable_names(), str.substr(0, str.find('(')))].size;
	}
	
	clog::error("Variable does not exist.");
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
	
	clog::error("Register does not exist.");
	return -1;
}

int get_size_of_number(const std::string &str) {
	long long number = std::stoll(str);	

	if (number < CHAR_MAX) return 1;
	if (number < SHRT_MAX) return 2;
	if (number < INT_MAX) return 4;
	if (number < LONG_MAX) return 8;

	clog::error("Number out of bounds.");
	return 0;
}

int get_size_of_operand(std::string str, int number_default = -1) {
	auto names = variable_names();
	int variable_index = index_of(names, str);
	if (is_dereferenced(str)) {
		str.erase(str.begin());
		str.pop_back();
		if (RegisterRef reg = get_register(str); reg.has_value()) {
			auto corresponding_variable_it = std::ranges::find_if(dereferenced_register_variable_correspondant, [&](auto p){ return (p.first == str); });
			variable_index = index_of(names, corresponding_variable_it->second);
		}
		return variables[variable_index].true_pointer_size;
	} if (is_variable(str)) {
		return get_size_of_asm_variable(str);
	} else if (is_number(str)) {
		if (number_default != -1) return number_default;
		return get_size_of_number(str);
	} else if (RegisterRef reg = get_register(str); reg.has_value()) {
		return get_size_of_register(str);
	} else if (auto it = std::ranges::find(names, str); it != names.end()) {
		return variables[variable_index].size;
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
	if (size != -1)
		clog::error("Invalid register size.");
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
	int lhs_size = get_size_of_operand(lhs, rhs_size);
	return get_mov_instruction(lhs_size, rhs_size);
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
			var_name = variables[index_of(variable_stack_locations(), std::stoi(before_parenthesis))].name;
		} else if (is_global_variable(*tok_it)) {
			var_name = before_parenthesis;
		} else {
			clog::error("Only pointers can be dereferenced");
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
		} else {
			clog::error("Invalid math operator.");
		}
		if (cmd == "add" || cmd == "sub") {
			int rhs_size = get_size_of_operand(*(tok_it-1));
			RegisterRef rhs = get_available_register(); // asm rhs (math output)
			rhs->get().occupied = true;
			std::string rhs_name =  rhs->get().name_from_size(rhs_size);
			out.push_back(mov(*(tok_it-1), rhs_name) + '\n');
			
			std::pair<std::string, std::string> lhs_rhs = cast_lhs_rhs(*(tok_it+1), rhs_name, 4);
			out.push_back(
				cmd + types::suffixes[index_of(types::sizes, get_size_of_operand(lhs_rhs.second))] + ' ' +
				set_operand_prefix(lhs_rhs.first) + ", " + set_operand_prefix(lhs_rhs.second) + '\n'
			);
			//size_t old_it_index = from_it(_us_ltoks, tok_it);
			commit(replace_toks(_us_ltoks, tok_it-1, tok_it+1, lhs_rhs.second));
			tok_it -= 1; // Tok it is out of bounds since "[x] [+] [y]" narrows down to "[result]"
			
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
			variables.back().is_pointer = true;
			variables.back().true_pointer_size = types::sizes[type_vec_index];
		} else if (current_function.empty()) {
			out.insert(DATA_ASM+1, ".globl " + name + '\n');
			out.insert(DATA_ASM+2, ".align 8\n");
			out.insert(DATA_ASM+3, ".type " + name + ", @object\n");
			out.insert(DATA_ASM+4, ".size " + name + ", " + std::to_string(types::sizes[type_vec_index]) + '\n');
			out.insert(DATA_ASM+5, name + ":\n");
			
			out.insert(DATA_ASM+6, types::asm_types[type_vec_index] + ' ' + value + '\n');
			
			variables.push_back(Variable{name, types::sizes[type_vec_index], INT_MAX, false, -1, braces::braces_end_index()});
		} else {
			current_stack_size += types::sizes[type_vec_index];
			variables.push_back(Variable{name, types::sizes[type_vec_index], -current_stack_size, false, -1, braces::braces_end_index()});
			std::pair<std::string, std::string> lhs_rhs = cast_lhs_rhs(value, name);
			std::string str = get_mov_instruction(lhs_rhs.first, lhs_rhs.second) + ' ';
			str += set_operand_prefix(lhs_rhs.first) + ", -" +  std::to_string(current_stack_size) + "(%rbp)\n";
			out.push_back(str);
			unoccupy_if_register(lhs_rhs.first);
			unoccupy_if_register(lhs_rhs.second);
		}
	}

	void variable_declaration(TokIt tok_it, const std::string &current_function, int &current_stack_size) {
		bool is_pointer = false;
		if (tok_it != _us_ltoks.begin()) {
			is_pointer = (*(tok_it-1) == "^^");
		}
		
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
			out.push_back("movl " + SYS_WRITE + ", %eax" + '\n');
			//equals(*(tok_it+2), get_register("%rdi")->get().name_from_size(get_size_of_operand(*(tok_it+2))));
			//equals(*(tok_it+3), get_register("%rsi")->get().name_from_size(get_size_of_operand(*(tok_it+3))));
			//equals(*(tok_it+4), get_register("%rdx")->get().name_from_size(get_size_of_operand(*(tok_it+4))));
			equals(*(tok_it+2), "%edi");
			equals(*(tok_it+3), "%rsi");
			equals(*(tok_it+4), "%edx");
			out.push_back("syscall\n");
			unoccupy_if_register(*(tok_it+2));
			unoccupy_if_register(*(tok_it+3));
			unoccupy_if_register(*(tok_it+4));
		} else if (*(tok_it+1) == "r") { // READ
			out.push_back("movl " + SYS_READ + ", %eax" + '\n');
			//equals(*(tok_it+2), get_register("%rdi")->get().name_from_size(get_size_of_operand(*(tok_it+2))));
			//equals(*(tok_it+3), get_register("%rsi")->get().name_from_size(get_size_of_operand(*(tok_it+3))));
			//equals(*(tok_it+4), get_register("%rdx")->get().name_from_size(get_size_of_operand(*(tok_it+4))));
			equals(*(tok_it+2), "%edi");
			equals(*(tok_it+3), "%rsi");
			equals(*(tok_it+4), "%edx");
			out.push_back("syscall\n");
			unoccupy_if_register(*(tok_it+2));
			unoccupy_if_register(*(tok_it+3));
			unoccupy_if_register(*(tok_it+4));
		} else if (*(tok_it+1) == "e") { // EXIT
			out.push_back("movl " + SYS_EXIT + ", %eax" + '\n');
			//equals(*(tok_it+2), get_register("%rdi")->get().name_from_size(get_size_of_operand(*(tok_it+2))));
			equals(*(tok_it+2), "%edi");
			out.push_back("syscall\n");
			unoccupy_if_register(*(tok_it+2));
		}
	}

	void function_declaration(TokIt tok_it, std::vector<std::string> &functions, std::vector<int> &stack_sizes, std::string &current_function) {
		if (TEXT_ASM == out.end()) {
			out.push_back(".text\n");
		}
		
		std::vector<std::string> attributes{};
		std::copy(_us_ltoks.begin(), tok_it, attributes.begin());
		
		
		
		std::string func_name = *(tok_it+1);
		out.push_back(".globl " + func_name + '\n');
		out.push_back(".type " + func_name + ", @function\n");
		out.push_back(func_name + ":\n");
		out.push_back("pushq %rbp\n");
		out.push_back("movq %rsp, %rbp\n");
		//out.push_back("subq $16, %rsp\n");

		functions.push_back(func_name);
		stack_sizes.push_back(0);
		current_function = func_name;
		
		braces::braces.push_back(Brace(Brace::State::Open, Brace::Type::Function, braces::braces.back().index+1, -1));
	}
	
	void function_call(TokIt tok_it) {
		out.push_back("movl $0, %eax\n");
		out.push_back("call " + *tok_it + '\n');
		commit(replace_tok(_us_ltoks, tok_it, "%eax"));
	}
	
	void function_return(TokIt tok_it, const std::vector<std::string> &toks, std::string &current_function) {
		size_t type_vec_index = index_of(types::sizes, get_size_of_operand(*(tok_it+1)));
		std::string ret = "mov" + types::suffixes[type_vec_index];
		RegisterRef rax = get_register("rax");
		ret += ' ' + set_operand_prefix(*(tok_it+1)) + ", " + rax->get().name_from_size(types::sizes[type_vec_index]) + '\n';
		out.push_back(ret);
		out.push_back("leave\n");
		out.push_back("ret\n");
	}
	
	void function_end(TokIt tok_it) {
		out.push_back(".size " + current_function + ", .-" + current_function + '\n');
		current_function = "";
	}

	std::string condition_operator_to_asm(std::string op, bool reverse = false) {
		if (reverse) {
			if (op == "==") {
				return "ne";
			} else if (op == "!=") {
				return "e";
			} else if (op == "<") {
				return "ge";
			} else if (op == "<=") {
				return "g";
			} else if (op == ">") {
				return "le";
			} else if (op  == ">=") {
				return "l";
			}
		} else {
			if (op == "==") {
				return "e";
			} else if (op == "!=") {
				return "ne";
			} else if (op == "<") {
				return "l";
			} else if (op == "<=") {
				return "le";
			} else if (op == ">") {
				return "g";
			} else if (op  == ">=") {
				return "ge";
			}
		}
		
		clog::error("Invalid condition operator.");
		return "";
	}
	
	void conditional(std::string rhs, std::string lhs, std::string con, int label) {
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

		out.push_back('j' + con + " .L" + std::to_string(label) + '\n');
	}
	
	void if_statement(TokIt tok_it) {
		braces::braces.push_back(Brace(Brace::State::Open, Brace::Type::If, braces::braces_begin_index(), braces::get_last_condition(true).type_index+1));
		conditional(*(tok_it-3), *(tok_it-1), condition_operator_to_asm(*(tok_it-2), true), braces::braces.back().type_index);
	}

	void else_statement(TokIt tok_it) {
		if (tok_it+1 == _us_ltoks.end() || _us_ltoks.back() != "?") braces::braces.push_back(Brace(Brace::State::Open, Brace::Type::Else, braces::braces_begin_index(), braces::get_last_condition(true).type_index+1));
		out.insert(out.end()-1, "jmp .L" + std::to_string(braces::braces.back().type_index) + '\n');
	}
	
	void while_loop(TokIt tok_it) {
		braces::braces.push_back(Brace(Brace::State::Open, Brace::Type::While, braces::braces_begin_index(), braces::get_last_condition(true).type_index+2));
		out.push_back("jmp .L" + std::to_string(braces::braces.back().type_index-1) + '\n');
		out.push_back(".L" + std::to_string(braces::braces.back().type_index) + ":\n");
		conditional(*(tok_it-3), *(tok_it-1), condition_operator_to_asm(*(tok_it-2)), braces::braces.back().type_index);
	}
	
	void while_loop_end(TokIt tok_it) {	
		// Move condition from beginning of loop to end
		// This is the worst code I've ever written in my life

		// Find boundaries of condition
		std::vector<std::string> reverse_vec = out;
		std::ranges::reverse(reverse_vec);
		std::vector<std::string>::iterator begin = std::ranges::find(reverse_vec, (".L" + std::to_string(braces::braces_end_index(true)) + ":\n"))-1;
		auto original_bound_begin = out.end()-from_it(reverse_vec, begin)-1;
		
		auto is_end = [](const std::string &str){ return (str[0] == 'j' && str.find(".L") != std::string::npos); };
		auto end = std::ranges::find_if(original_bound_begin, out.end(), is_end)+1;
		
		out.push_back(".L" + std::to_string(braces::braces_end_index(true)-1) + ":\n");
	
		size_t begin_i = from_it(out, original_bound_begin);
		size_t end_i = from_it(out, end);
		std::vector<std::string> range{};
		for (auto it = original_bound_begin; it != end; it++) range.push_back(*it);
		out.insert(out.end(), range.begin(), range.end());
		out.erase(out.begin()+begin_i, out.begin()+end_i);
	}
	
	void brace_open(TokIt tok_it) {
		braces::braces.push_back(Brace(Brace::State::Open, Brace::Type::General, braces::braces_begin_index(), -1));
	}
	
	void if_end(TokIt tok_it) {
		out.push_back(".L" + std::to_string(braces::braces_end_index(true)) + ":\n");
	}
	
	void brace_close(TokIt tok_it) {
		if (braces::braces.size() > 1) {
			int index = braces::braces_end_index(false);
			Brace open_brace = braces::get_last_open_index(index);
			int type_index = -1;
			if (open_brace.index != -1) {
				if (open_brace.type == Brace::Type::Function) {
					function_end(tok_it);
				} else if (open_brace.type == Brace::Type::If || open_brace.type == Brace::Type::Else) {
					if_end(tok_it);
				} else if (open_brace.type == Brace::Type::While) {
					while_loop_end(tok_it);
				}
				braces::braces.push_back(Brace(Brace::State::Close, open_brace.type, index, open_brace.type_index));
				
				for (int i = 0; i < variables.size(); i++) {
					//for (int i = from_it(braces::braces, std::find(braces::braces.rbegin(), braces::braces.rend(), open_brace).base()); i < braces::braces.size(); i++) {
					if (variables[i].braces_index == index && variables[i].stack_location < 0) {
						variables.erase(variables.begin()+i);
					}
				}
				
				return;
			}
		}
		clog::error("Closing brace unexpected.");
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
			variables.push_back(Variable{".STR" + std::to_string(str_index), 8, INT_MAX, false, -1, braces::braces_end_index()});
			str_index++;
		}
		in_quote = !in_quote;
	}
	
}

int begin_compile(std::vector<std::string> args) {
	try {

	std::string output_dir = "./";
	if (args.size() < 2) {
		clog::error("Must input file to compile.", 0);
		return 0;
	} else if (auto it = std::ranges::find(args, "-od"); it != args.end()) {
		output_dir = *(it+1);
		if ((it+1)->back() != '/') output_dir += '/';
	}

	std::ofstream write;
	write.open(output_dir + "rcout.s", std::ofstream::out | std::ios::trunc);
	
	std::vector<std::string> disallowed_toks = { };

	std::vector<std::string> functions = { };
	std::vector<int> current_function_stack_sizes = { };

	int str_index = 0;
	bool in_quote = false;

	registers.push_back(Register());	
	
	line_number = 0;
	for (std::string l : lines) {
		line_number++;
		l = trim(l);
		_ltoks = split(l);
		_us_ltoks = unspaced(_ltoks);
		
		//std::ranges::for_each(_us_ltoks,[](auto s){std::cout<<s<<' ';});std::cout<<std::endl; // print all toks
		
		while_us_find_token("\"", 0, 0, [&](TokIt tok_it) {
			token_function::quote(tok_it, str_index, in_quote);
		});
		while_us_find_token("//", 0, 0, [&](TokIt tok_it) {
			int i = std::distance(_us_ltoks.begin(), tok_it);
			while ( i < _us_ltoks.size()) {
				_us_ltoks.erase(_us_ltoks.begin()+i);
			}
		});
		while_us_find_token("#", 0, 1, [&](TokIt tok_it) {
			if (tok_it != _us_ltoks.begin()) {
				if (std::ranges::find(functions, *(tok_it+1)) != functions.end()) {
					clog::error("Cannot declare function. Function of this name already exists.");
					return;
				}
			}
			token_function::function_declaration(tok_it, functions, current_function_stack_sizes, current_function);
		});
		while_us_find_tokens(functions, 0, 0, [&](TokIt tok_it) {
			if (std::distance(_us_ltoks.begin(), tok_it) > 0) {
				if (*(tok_it-1) == "#") {
					return;
				}
			}
			token_function::function_call(tok_it);
		});
		while_us_find_tokens(variable_names(), 0, 0, [&](TokIt tok_it) {
			if (tok_it != _us_ltoks.begin()) {
				if (std::ranges::find(types::types, *(tok_it-1)) != types::types.end()) {
					clog::error("Cannot define variable. Variable of this name already exists.");
					return;
				}
			}
	
			size_t vec_index = index_of(variable_names(), *tok_it);
			if (variable_stack_locations()[vec_index] != INT_MAX) {
				commit(replace_tok(_us_ltoks, tok_it, std::to_string(variable_stack_locations()[vec_index]) + "(%rbp)"));
			} else {
				*tok_it += "(%rip)";
				commit(_us_ltoks);
			}
		});
		while_us_find_token("^", 0, 1, [&](TokIt tok_it) {
			token_function::dereference(tok_it);
		});
		while_us_find_token("&", 0, 1, [&](TokIt tok_it) {
			token_function::address_of(tok_it);
		});
		while_us_find_tokens(math_symbols, 1, 1, [&](TokIt tok_it) {
			token_function::math(tok_it);
		});
		while_us_find_token("=", 1, 1, [&](TokIt tok_it) {
			token_function::equals(tok_it);
		});
		while_us_find_tokens(types::types, 0, 1, [&](TokIt tok_it) {
			size_t func_vec_index = from_it(functions, std::ranges::find(functions, current_function));
			token_function::variable_declaration(tok_it, current_function, current_function_stack_sizes[func_vec_index]);
		});
		while_us_find_token("#>", 0, 1, [&](TokIt tok_it) {
		token_function::function_return(tok_it, _us_ltoks, current_function);
		});
		while_us_find_token("{", 0, 0, [&](TokIt tok_it) {
			token_function::brace_open(tok_it);
		});
		while_us_find_token("}", 0, 0, [&](TokIt tok_it) {
			token_function::brace_close(tok_it);
		});
		while_us_find_token("??", 0, 0, [&](TokIt tok_it) {
			token_function::else_statement(tok_it);
		});
		while_us_find_token("?", 1, 0, [&](TokIt tok_it) {
			token_function::if_statement(tok_it);
		});
		while_us_find_token("*?", 1, 0, [&](TokIt tok_it) {
			token_function::while_loop(tok_it);
		});
		while_us_find_token("~", 0, 0, [&](TokIt tok_it) {
			commit(replace_tok(_us_ltoks, tok_it, "rax"));
		});
		while_us_find_token(">", 0, 2, [&](TokIt tok_it) {
			token_function::base_functions(tok_it);
		});

		disallowed_toks.clear();

		for (Register &reg : registers) {
			reg.occupied = false;
		}
	}

	out.insert(out.begin(), ".file \"" + std::string(args[1]) + "\"\n");

	for (std::string &str : out) {
		if (str.find(':') == std::string::npos && str[0] != '.') {
			str.insert(str.begin(), '\t');
		}

		write << str;
	}
	
	write.close();
	
	system(((std::string)"as " + output_dir + "rcout.s -o " + output_dir + "rcout.o && ld " + output_dir + "rcout.o -e main -o " + output_dir + "a.out && " + output_dir + "a.out").c_str());
	
	} catch (std::string &e) {
		std::cerr << e << std::endl;
		std::cerr << "Aborting." << std::endl;
	}

	return 0;
}

