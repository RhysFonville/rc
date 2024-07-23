#include <algorithm>
#include <functional>
#include <iostream>
#include <fstream>
#include <string>
#include <climits>
#include <optional>
#include <ranges>
#include <utility>
#include <regex>
#include "preprocessor.h" // Including in here stops weird redefinition errors for util.h

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
	
	static Brace & get_last(std::vector<Brace::Type> types, bool open = false) {
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
	
	static Brace & get_last_condition(bool open = false) {
		return get_last({ Brace::Type::If, Brace::Type::Else, Brace::Type::While }, open);
	}
	
	static Brace & get_last_index(int index) {
		for (Brace &brace : braces | std::views::reverse) {
			if (brace.index == index) {
				return brace;
			}
		}
		return braces.front();
	}
	
	static Brace & get_last_open_index(int index) {
		for (Brace &brace : braces | std::views::reverse) {
			if (brace.state == Brace::State::Open && brace.index == index) {
				return brace;
			}
		}
		return braces.front();
	}
	
	static int braces_begin_index(bool type = false) {
		if (braces::braces.back().state == Brace::State::Close) {
			return braces::braces.back().index;
		} else {
			return braces::braces.back().index+1;
		}
	}

	static int braces_end_index(bool type = false) {
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

struct Type {
	std::string name{};
	std::string asm_name{};
	int size{};
	char suffix{};
	bool is_pointer{};
	int true_pointer_size{};
	
	Type() {}
	Type(const std::string& name, const std::string& asm_name, int size, char suffix)
		: name{name}, asm_name{asm_name}, size{size}, suffix{suffix} {}

	bool operator==(const Type& type) {
		return (
			name == type.name &&
			is_pointer == type.is_pointer
		);
	}

	bool operator!=(const Type& type) {
		return !(*this == type);
	}
/*
	void operator=(const Type& type) {
		name = type.name;
		asm_name = type.asm_name;
		size = type.size;
		suffix = type.suffix;
		is_pointer = type.is_pointer;
		true_pointer_size = type.true_pointer_size;
	}
*/
};

static std::vector<Type> types{
	{"int", ".word", sizeof(int), 'l'},
	{"lng", ".long", sizeof(long), 'q'},
	{"sht", ".short", sizeof(short), 'w'},
	{"ch", ".byte", sizeof(char), 'b'}
};

static std::string get_number(std::string s) {
    for (Type type : types) {
		if (int i = s.find(type.name); i != std::string::npos) {
			s = s.substr(0, i);
		}
	}

	return s;
}

static bool is_number(std::string s) {
	s = get_number(s);
	std::string::const_iterator it = s.begin();
    while (it != s.end() && (std::isdigit(*it) || *it == '-')) ++it;
    return !s.empty() && it == s.end();
}

static bool is_stack_variable(const std::string &str) {
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

static bool is_global_variable(const std::string &str) {
	if (str.find("%rip") != std::string::npos) {
		return true;
	}
	return false;
}

static bool is_variable(const std::string &str) {
	return (is_stack_variable(str) || is_global_variable(str));
}

static bool is_dereferenced(const std::string &str) {
	return (str.front() == '(' && str.back() == ')');
}

static std::vector<std::string> type_names() {
	return types | std::views::transform([](const Type& type) { return type.name; }) | std::ranges::to<std::vector>();
}

static std::optional<Type> get_type_of_size(int size) {
	for (const Type& type : types) {
		if (type.size == size) {
			return type;
		}
	}

	return std::nullopt;
}

static std::optional<Type> get_type_of_name(const std::string& name) {
	for (const Type& type : types) {
		if (type.name == name) {
			return type;
		}
	}

	return std::nullopt;
}

struct Variable {
	std::string name;
	Type type{};
	int stack_location;
	int braces_index;
	std::vector<std::string> type_qualifiers;
};

static std::vector<Variable> variables;

static std::vector<std::pair<std::string, Type>> dereferenced_type_correspondant{};
static std::vector<std::pair<std::string, Type>> addressed_type_correspondant{};

static std::vector<std::string> variable_names() {
	std::vector<std::string> names{};
	for (Variable &var : variables) {
		names.push_back(var.name);
	}
	return names;
}

static std::vector<int> variable_stack_locations() {
	std::vector<int> stack_locations{};
	for (Variable &var : variables) {
		stack_locations.push_back(var.stack_location);
	}
	return stack_locations;
}

static std::vector<Type> variable_types() {
	std::vector<Type> types{};
	for (Variable &var : variables) {
		types.push_back(var.type);
	}
	return types;
}

static std::vector<int> variable_braces_index() {
	std::vector<int> braces_indices{};
	for (Variable &var : variables) {
		braces_indices.push_back(var.braces_index);
	}
	return braces_indices;
}

static std::vector<std::vector<std::string>> variable_type_qualifiers() {
	std::vector<std::vector<std::string>> type_qualifiers{};
	for (Variable &var : variables) {
		type_qualifiers.push_back(var.type_qualifiers);
	}
	return type_qualifiers;
}

static std::optional<std::reference_wrapper<Variable>> get_variable_by_name(const std::string &name) {
	if (!is_variable(name)) {
		return std::nullopt;
	}

	for (Variable &var : variables) {
		if (var.name == name) return std::optional<std::reference_wrapper<Variable>>{var};
	}

	return std::nullopt;
}

static std::optional<std::reference_wrapper<Variable>> get_variable_by_asm(const std::string &str) {
	if (!is_variable(str)/* && !is_dereferenced(str)*/) {
		return std::nullopt;
	}

	/*
	if (is_dereferenced(str)) {
		for (std::pair<std::string, std::string> reg_var : dereferenced_register_variable_correspondant) {
			if (reg_var.first == str.substr(1, str.size()-2)) {
				for (Variable &var : variables) {
					if (var.name == reg_var.second) return std::optional<std::reference_wrapper<Variable>>{var};
				}
			}
		}
	}
	*/
	
	for (Variable &var : variables) {
		std::string before_parenthesis = str.substr(0, str.find('('));
		if (is_global_variable(str)) {
			if (var.name == before_parenthesis) return std::optional<std::reference_wrapper<Variable>>{var};
		} else if (is_stack_variable(str)) {
			if (var.stack_location == std::stoi(before_parenthesis)) return std::optional<std::reference_wrapper<Variable>>{var};
		}
	}

	return std::nullopt;
}

static std::optional<std::reference_wrapper<Variable>> get_variable_by_asm_or_name(const std::string &str) {
	std::optional<std::reference_wrapper<Variable>> name = get_variable_by_name(str);
	std::optional<std::reference_wrapper<Variable>> asm_var = get_variable_by_asm(str);
	
	if (name.has_value()) {
		return name;
	} else if (asm_var.has_value()) {
		return asm_var;
	} else {
		return std::nullopt;
	}
}

static std::string current_function = "";

static std::vector<Register> registers = {
	Register("rbx","ebx","bx","bh","bl"), Register("r10","r10d","r10w","r10b","r10b"), Register("r11","r11d","r11w","r11b","r11b"), Register("r12","r12d","r12w","r12b","r12b"), Register("r13","r13d","r13w","r13b","r13b"), Register("r14","r14d","r13w","r13b","r13b"), Register("r15","r15d","r15w","r15b","r15b"),
	Register("rax","eax","ax","ah","al"), Register("rdi","edi","di","dil","dil"), Register("rsi","esi","si","sil","sil"), Register("rdx","edx","dx","dl","dh"), Register("rcx","ecx","cx","ch","cl"), Register("r8","r8d","r8w","r8b","r8b"), Register("r9","r8d","r8w","r8b","r8b"), // SYSCALL REGISTERS
	Register("rsp","esp","sp","spl","spl"), Register("rbp","ebp","bp","bpl","bpl") // STACK REGISTERS
};

static const std::vector<std::string> math_symbols = { "*", "/", "+", "-" };

static std::vector<std::string> out;

static const std::string SYS_READ  = "$0";
static const std::string SYS_WRITE = "$1";
static const std::string SYS_EXIT  = "$60";
static const std::string STDIN     = "$0";
static const std::string STDOUT    = "$1";

static RegisterRef get_available_register() {
	for (Register &reg : registers) {
		if (!reg.occupied) return RegisterRef(reg); 
	}

	message::error("No registers are available.");
	return std::nullopt;
}

static RegisterRef get_register(std::string str) {
	for (Register &reg : registers) {
		if (reg.comp_names(reg.names.q, str) ||
			reg.comp_names(reg.names.l, str) ||
			reg.comp_names(reg.names.w, str) ||
			reg.comp_names(reg.names.bh, str) ||
			reg.comp_names(reg.names.bl, str)) return RegisterRef(reg);
	}
	
	return std::nullopt;
}

static std::string get_string_literal(const std::vector<std::string> &toks, TokIt index, bool with_quotes = true) { // Not actually used but I'm keeping it
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

//bool is_pointer(const std::string &str) {
//	return str.find("(%rip)") != std::string::npos;
//}

static std::string prep_asm_str(std::string str) {
	if (is_number(str)) {
		str = get_number(str);
	}

	if (!get_register(str).has_value() && str.back() != ')') {
		str = '$' + str;
	}

	return str;
}

static int get_size_of_asm_variable(const std::string &str) {
	if (str.find("%rbp") != std::string::npos) {
		int stack_offset = std::stoi(str.substr(1, str.find('(')-1));
		int var_vec_index = index_of(variable_stack_locations(), -stack_offset);
		
		return variables[var_vec_index].type.size;
	} else if (str.find("%rip") != std::string::npos) {
		return variables[index_of(variable_names(), str.substr(0, str.find('(')))].type.size;
	}
	
	message::error("Variable does not exist.");
	return -1;
}

static int get_size_of_register(const std::string &str) {
	for (Register &reg : registers) {
		if (reg.comp_names(reg.names.q, str)) return 8;
		if (reg.comp_names(reg.names.l, str)) return 4;
		if (reg.comp_names(reg.names.w, str)) return 2;
		if (reg.comp_names(reg.names.bh, str)) return 1;
		if (reg.comp_names(reg.names.bl, str)) return 1;
	}
	
	message::error("Register does not exist.");
	return -1;
}

static int get_size_of_number(const std::string &str) {
	int i = 0;
	for (const std::string& type_name : type_names()) {
		if (str.find(type_name) != std::string::npos) {
			return types[i].size;
		}
		i++;
	}

	long long number = std::stoll(str);	
	
	if (number < CHAR_MAX) return 1;
	if (number < SHRT_MAX) return 2;
	if (number < INT_MAX) return 4;
	if (number < LONG_MAX) return 8;

	message::error("Number out of bounds.");
	return 0;
}

static Type get_type(const std::string &str) {
	std::optional<std::reference_wrapper<Variable>> var{get_variable_by_asm_or_name(str)};
	if (var.has_value()) {
		return var->get().type;
	} else if (is_number(str)) {
		std::optional<Type> type{get_type_of_size(get_size_of_number(str)).value()};
		if (!type.has_value()) {
			message::error("Can't get type for number.");
		}
		return type.value();
	} else if (auto it = std::ranges::find_if(dereferenced_type_correspondant, [&](auto p){ return p.first == str; }); it != dereferenced_type_correspondant.end()) {
		return it->second;
	} else if (auto it = std::ranges::find_if(addressed_type_correspondant, [&](auto p){ return p.first == str; }); it != addressed_type_correspondant.end()) {
		return it->second;
	} else {
		message::error("Can't get type");
	}
	
	return Type{};
}

/*
[[deprecated("Use get_type")]]
static int get_size_of_operand(std::string str, int number_default = -1) {
	auto names = variable_names();
	int variable_index = index_of(names, str);
	if (is_dereferenced(str)) {
		str.erase(str.begin());
		str.pop_back();
		if (RegisterRef reg = get_register(str); reg.has_value()) {
			std::vector<std::pair<std::string, Type>>::iterator corresponding_type_it = std::ranges::find_if(dereferenced_type_correspondant, [&](auto p){ return (p.first == str); });
			return corresponding_type_it->second.size;
		}
		return variables[variable_index].type.true_pointer_size;
	} if (is_variable(str)) {
		return get_size_of_asm_variable(str);
	} else if (is_number(str)) {
		if (number_default != -1) return number_default;
		return get_size_of_number(str);
	} else if (RegisterRef reg = get_register(str); reg.has_value()) {
		return get_size_of_register(str);
	} else if (auto it = std::ranges::find(names, str); it != names.end()) {
		return variables[variable_index].type.size;
	} else {
		return -1;
	}
} 
*/

static char size_to_letter(int size) {
	if (size == 1)
		return 'b';
	else if (size == 2)
		return 'w';
	else if (size == 4)
		return 'l';
	else if (size == 8)
		return 'q';
	if (size != -1)
		message::error("Invalid register size.");
	return '\0';
}

static std::string sign_extension_mov(int lhs, int rhs) {
	return std::string("movs") + size_to_letter(lhs) + size_to_letter(rhs);
}

static std::string zero_extension_mov(int lhs, int rhs) {
	return std::string("movz") + size_to_letter(lhs) + size_to_letter(rhs);
}

static std::string get_mov_instruction(int lhs, int rhs) {
	if (lhs < rhs)
		return sign_extension_mov(lhs, rhs);
	else
		return std::string{"mov"} + (rhs != -1 ? get_type_of_size(rhs)->suffix : 'q');
}

static std::string get_mov_instruction(const std::string &lhs, const std::string &rhs) {
	Type rhs_type = get_type(rhs);
	Type lhs_type = get_type(lhs);
	return get_mov_instruction((lhs_type.true_pointer_size != -1 ? lhs_type.true_pointer_size : lhs_type.size),
							(rhs_type.true_pointer_size != -1 ? rhs_type.true_pointer_size : rhs_type.size));
}

static std::string mov(const std::string &lhs, const std::string &rhs) {
	return get_mov_instruction(lhs, rhs) + ' ' + prep_asm_str(lhs) + ", " + prep_asm_str(rhs);
}

static void unoccupy_if_register(const std::string &reg_name) {
	if (RegisterRef reg = get_register(reg_name); reg.has_value()) {
		reg->get().occupied = false;
	}
}

/*
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
			out.push_back("movl " + prep_asm_str(lhs) + ", %eax\n");
			out.push_back("cltq\n");
			
			lhs = "%rax"; // Using 'rax' is probably a problem. Do I care right now though? No.
		} else if (lhs_size == 1 && max_size == 4) { // byte -> int
			RegisterRef reg = get_available_register();
			out.push_back("movsbl " + prep_asm_str(lhs) + ", " + reg->get().name_from_size(4) + '\n');
			lhs = reg->get().name_from_size(4);
		} else {
			RegisterRef reg = get_available_register();
			reg->get().occupied = true;
			out.push_back(get_mov_instruction(lhs_size, max_size) + ' ' + prep_asm_str(lhs) + ", " + reg->get().name_from_size(max_size) + '\n');
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
*/

static std::smatch get_innermost_parentheses(const std::string& str) {
	std::regex regex{R"(\([^\(]*?\))"};
	std::smatch match{};
	std::regex_search(str, match, regex);
	
	if (match.empty()) {
		std::regex entire_regex{R"(.+)"};
		std::regex_search(str, match, entire_regex);
	}

	return match;
}

namespace token_function {
	static void dereference(TokIt &tok_it) {
		_us_ltoks.erase(tok_it);
		std::string var_name = "";
		std::string before_parenthesis = tok_it->substr(0, tok_it->find('('));
		if (is_stack_variable(*tok_it)) {
			var_name = variables[index_of(variable_stack_locations(), std::stoi(before_parenthesis))].name;
		} else if (is_global_variable(*tok_it)) {
			var_name = before_parenthesis;
		} else {
			message::error("Only pointers can be dereferenced");
		}

		RegisterRef reg = get_available_register();
		reg->get().occupied = true;
		std::string reg_name = reg->get().name_from_size(8);
		out.push_back("movq " + *(tok_it) + ", " + reg_name + '\n');
		commit(replace_tok(_us_ltoks, tok_it, reg_name));
		commit(replace_tok(_us_ltoks, tok_it, '(' + *tok_it + ')'));
		dereferenced_type_correspondant.push_back(std::make_pair(reg_name, get_variable_by_name(var_name)->get().type));
	}

	static void address_of(TokIt tok_it) {
		RegisterRef reg = get_available_register();
		out.push_back("leaq " + *(tok_it+1) + ", " + reg->get().names.q + '\n');
		
		commit(replace_toks(_us_ltoks, tok_it, tok_it+1, reg->get().names.q));
	}

	static void math(TokIt &tok_it) {
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
			message::error("Invalid math operator.");
		}
		if (cmd == "add" || cmd == "sub") {
			Type lhs_type = get_type(*(tok_it-1));
			Type rhs_type = get_type(*(tok_it+1));

			if (lhs_type != rhs_type) {
				message::error("Mismatched types in arithmatic.");
			}
			
			if (is_number(*(tok_it-1)) && is_number(*(tok_it+1))) {
				commit(
					replace_toks(_us_ltoks, tok_it-1, tok_it+1,
						std::to_string(std::stoll(get_number(*(tok_it-1))) + std::stoll(get_number(*(tok_it+1)))) + lhs_type.name
					)
				);
				tok_it -= 1;
				unoccupy_if_register(*(tok_it+1));
				return;
			}
			
			// Move rhs into temp register
			RegisterRef lhs = get_available_register(); // code lhs (math output)
			lhs->get().occupied = true;
			std::string lhs_name = lhs->get().name_from_size(lhs_type.size);
			out.push_back(mov(*(tok_it-1), lhs_name) + '\n');
			
			out.push_back(
				cmd + rhs_type.suffix + ' ' + prep_asm_str(*(tok_it+1)) + ", " + prep_asm_str(*(tok_it-1)) + '\n'
			);
		
			commit(replace_toks(_us_ltoks, tok_it-1, tok_it+1, *(tok_it-1)));
			tok_it -= 1; // Tok it is out of bounds since "[x] [+] [y]" narrows down to "[result]"
			
			lhs->get().occupied = false;
			unoccupy_if_register(*(tok_it+1));
			
		} else if (cmd == "mul" || cmd == "div") {
			message::error("Multiplication and division not yet supported.");
			
			/*
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
			lhs_str += prep_asm_str(*(tok_it-1)) +  ", " + lhs->get().name_from_size(lhs_size)  + '\n';
			std::string rhs_str = get_mov_instruction(rhs_size, arithmatic_size) + ' ';
			rhs_str += prep_asm_str(*(tok_it+1)) +  ", " + rhs->get().name_from_size(rhs_size) + '\n';
			
			out.push_back(lhs_str);
			out.push_back(rhs_str);
			out.push_back(cmd + get_type_of_size(arithmatic_size)->suffix + ' ' + rhs->get().name_from_size(arithmatic_size) + '\n');

			commit(replace_toks(_us_ltoks, tok_it-1, tok_it+1, lhs->get().name_from_size(arithmatic_size)));

			lhs->get().occupied = true;
			rhs->get().occupied = false;
			*/
		}
	}

	static void cast(TokIt tok_it) {
		std::optional<Variable> var = get_variable_by_asm(*(tok_it-1));
		if (var.has_value()) {
			var->type = get_type_of_name(*(tok_it+1)).value();
		} else if (is_number(*(tok_it-1))) {
			commit(replace_toks(_us_ltoks, tok_it-1, tok_it+1, get_number(*(tok_it-1)) + *(tok_it+1)));
			tok_it -= 1;
		} else {
			message::error("Invalid token to cast.");	
		}
	}

	static void variable_declaration(const std::string &type, const std::string &name, std::string value,
						   const std::string &current_function, int &current_stack_size, bool is_pointer) {
		std::optional<Type> _type = get_type_of_name(type);
		if (!_type.has_value()) {
			message::error("Type does not exist.");
		}

		if (DATA_ASM == out.end()) {
			out.insert(TEXT_ASM, ".data\n");
		}
		
		if (is_pointer) {
			variable_declaration("lng", name, value, current_function, current_stack_size, false);
			variables.back().type.is_pointer = true;
			variables.back().type.true_pointer_size = _type->size;
		} else if (current_function.empty()) {
			out.insert(DATA_ASM+1, ".globl " + name + '\n');
			out.insert(DATA_ASM+2, ".align 8\n");
			out.insert(DATA_ASM+3, ".type " + name + ", @object\n");
			out.insert(DATA_ASM+4, ".size " + name + ", " + std::to_string(_type->size) + '\n');
			out.insert(DATA_ASM+5, name + ":\n");
			
			out.insert(DATA_ASM+6, _type->asm_name + ' ' + value + '\n');
			
			variables.push_back(Variable{name, _type.value(), INT_MAX, braces::braces_end_index()});
		} else {
			current_stack_size += _type->size;
			variables.push_back(Variable{name, _type.value(), -current_stack_size, braces::braces_end_index()});
			//std::pair<std::string, std::string> lhs_rhs = cast_lhs_rhs(value, name);
			//std::string str = get_mov_instruction(lhs_rhs.first, lhs_rhs.second) + ' ';
			//str += prep_asm_str(lhs_rhs.first) + ", -" +  std::to_string(current_stack_size) + "(%rbp)\n";
			//out.push_back(str);
			//unoccupy_if_register(lhs_rhs.first);
			//unoccupy_if_register(lhs_rhs.second);
			
			if (_type.value() != get_type(value)) {
				message::error("Cant set equal two different types.");
			}
			
			out.push_back(
				mov(value, std::to_string(variables.back().stack_location) + "(%rbp)") + '\n'
			);
		}
	}
	
	static void variable_declaration(TokIt tok_it, const std::string &current_function, int &current_stack_size) {
		bool is_pointer = false;
		if (tok_it != _us_ltoks.begin()) {
			is_pointer = (*(tok_it-1) == "^^");
		}
		
		std::string value = "0";
		if (tok_it+2 != _us_ltoks.end()) {
			value = *(tok_it+2);
		}
		
		variable_declaration(*tok_it, *(tok_it+1), value, current_function, current_stack_size, is_pointer);
		
		std::vector<std::string> type_qualifiers{_us_ltoks.begin(), tok_it};
		variables.back().type_qualifiers = type_qualifiers;
	}
	
	static void equals(const std::string &lhs, const std::string &rhs, bool change_reg_size = false) {
		//std::pair<std::string, std::string> lhs_rhs = cast_lhs_rhs(lhs, rhs, get_size_of_operand(rhs), change_reg_size);
		
		if (get_type(lhs) != get_type(rhs)) {
			message::error("Cant set equal two different types.");
		}
		
		out.push_back(mov(lhs, rhs) + '\n');
		
		//unoccupy_if_register(lhs_rhs.first);
		//unoccupy_if_register(lhs_rhs.second);
	}
	
	static void equals(TokIt tok_it) {
		std::optional<std::reference_wrapper<Variable>> var{get_variable_by_asm_or_name(*(tok_it-1))};
		if (var.has_value()) { // doesn't work since var is a stack variable so tok_it-1 is '-4(%rbp)', not 'x'
			if (std::ranges::find(var->get().type_qualifiers, "const") != var->get().type_qualifiers.end() && !is_dereferenced(*(tok_it-1))) {
				message::error("Can't change the value of a constant variable.");
			}
			//if (std::ranges::find(var->get().type_qualifiers, "dconst") != var->get().type_qualifiers.end() && is_dereferenced(*(tok_it-1))) {
			//	message::error("Can't change the dereferenced value of dereference constant variable.");
			//}
		}

		equals(*(tok_it+1), *(tok_it-1), true);
	}

	static void base_functions(TokIt tok_it) {
		if (*(tok_it+1) == "w") { // WRITE
			if (get_type(*(tok_it+2)).size != sizeof(int)) {
				message::error("Base function 'w' parameter 1 accepts an integer.");
			}
			if (!get_type(*(tok_it+3)).is_pointer) {
				message::error("Base function 'w' parameter 2 accepts a pointer.");
			}
			if (get_type(*(tok_it+4)).size != sizeof(int)) {
				message::error("Base function 'w' parameter 3 accepts an integer.");
			}

			out.push_back("movl " + SYS_WRITE + ", %eax\n");
			out.push_back("movl " + prep_asm_str(*(tok_it+2)) + "%edi\n");
			out.push_back("movl " + prep_asm_str(*(tok_it+3)) + "%rsi\n");
			out.push_back("movl " + prep_asm_str(*(tok_it+4)) + "%edx\n");
			out.push_back("syscall\n");
			unoccupy_if_register(*(tok_it+2));
			unoccupy_if_register(*(tok_it+3));
			unoccupy_if_register(*(tok_it+4));
		} else if (*(tok_it+1) == "r") { // READ
			if (get_type(*(tok_it+2)).size != sizeof(int)) {
				message::error("Base function 'r' parameter 1 accepts an integer.");
			}
			if (!get_type(*(tok_it+3)).is_pointer) {
				message::error("Base function 'r' parameter 2 accepts a pointer.");
			}
			if (get_type(*(tok_it+4)).size != sizeof(int)) {
				message::error("Base function 'r' parameter 3 accepts an integer.");
			}
			
			out.push_back("movl " + SYS_READ + ", %eax\n");
			out.push_back("movl " + prep_asm_str(*(tok_it+2)) + ", %edi\n");
			out.push_back("movl " + prep_asm_str(*(tok_it+3)) + ", %rsi\n");
			out.push_back("movl " + prep_asm_str(*(tok_it+4)) + ", %edx\n");
			out.push_back("syscall\n");
			unoccupy_if_register(*(tok_it+2));
			unoccupy_if_register(*(tok_it+3));
			unoccupy_if_register(*(tok_it+4));
		} else if (*(tok_it+1) == "e") { // EXIT
			if (get_type(*(tok_it+2)).size != sizeof(int)) {
				message::error("Base function 'e' accepts an integer.");
			}

			out.push_back("movl " + SYS_EXIT + ", %eax\n");
			out.push_back("movl " + prep_asm_str(*(tok_it+2)) + ", %edi\n");
			out.push_back("syscall\n");
			unoccupy_if_register(*(tok_it+2));
		}
	}

	static void function_declaration(TokIt tok_it, std::vector<std::string> &functions, std::vector<int> &stack_sizes, std::string &current_function) {
		std::string func_name = *(tok_it+1);
		if (std::ranges::find(functions, func_name) != functions.end()) {
			message::error("Function of name \"" + func_name + "\" already defined.");
		}
		
		if (TEXT_ASM == out.end()) {
			out.push_back(".text\n");
		}
		
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
	
	static void function_call(TokIt tok_it) {
		out.push_back("movl $0, %eax\n");
		out.push_back("call " + *tok_it + '\n');
		commit(replace_tok(_us_ltoks, tok_it, "%eax"));
	}
	
	static void function_return(TokIt tok_it, const std::vector<std::string> &toks, std::string &current_function) {
		Type type = get_type(*(tok_it+1));
		
		std::string ret = std::string{"mov"} + type.suffix;
		RegisterRef rax = get_register("rax");
		ret += ' ' + prep_asm_str(*(tok_it+1)) + ", " + rax->get().name_from_size(type.size) + '\n';
		out.push_back(ret);
		out.push_back("leave\n");
		out.push_back("ret\n");
	}
	
	static void function_end(TokIt tok_it) {
		out.push_back(".size " + current_function + ", .-" + current_function + '\n');
		current_function = "";
	}

	static std::string condition_operator_to_asm(std::string op, bool reverse = false) {
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
		
		message::error("Invalid condition operator.");
		return "";
	}
	
	static void conditional(std::string lhs, std::string rhs, std::string con, int label) {
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
		
		//std::pair<std::string, std::string> lhs_rhs = cast_lhs_rhs(lhs, rhs);
		
		Type type = get_type(lhs);
		
		// Insert before call instruction
		out.push_back(std::string{"cmp"} + type.suffix +
			' ' + prep_asm_str(lhs) + ", " + prep_asm_str(rhs) + '\n'
		);

		out.push_back('j' + con + " .L" + std::to_string(label) + '\n');
	}
	
	static void if_statement(TokIt tok_it) {
		braces::braces.push_back(Brace(Brace::State::Open, Brace::Type::If, braces::braces_begin_index(), braces::get_last_condition(true).type_index+1));
		conditional(*(tok_it-3), *(tok_it-1), condition_operator_to_asm(*(tok_it-2), true), braces::braces.back().type_index);
	}

	static void else_statement(TokIt tok_it) {
		if (tok_it+1 == _us_ltoks.end() || _us_ltoks.back() != "?") braces::braces.push_back(Brace(Brace::State::Open, Brace::Type::Else, braces::braces_begin_index(), braces::get_last_condition(true).type_index+1));
		out.insert(out.end()-1, "jmp .L" + std::to_string(braces::braces.back().type_index) + '\n');
	}
	
	static void while_loop(TokIt tok_it) {
		braces::braces.push_back(Brace(Brace::State::Open, Brace::Type::While, braces::braces_begin_index(), braces::get_last_condition(true).type_index+2));
		out.push_back("jmp .L" + std::to_string(braces::braces.back().type_index-1) + '\n');
		out.push_back(".L" + std::to_string(braces::braces.back().type_index) + ":\n");
		conditional(*(tok_it-3), *(tok_it-1), condition_operator_to_asm(*(tok_it-2)), braces::braces.back().type_index);
	}
	
	static void while_loop_end(TokIt tok_it) {	
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
	
	static void brace_open(TokIt tok_it) {
		braces::braces.push_back(Brace(Brace::State::Open, Brace::Type::General, braces::braces_begin_index(), -1));
	}
	
	static void if_end(TokIt tok_it) {
		out.push_back(".L" + std::to_string(braces::braces_end_index(true)) + ":\n");
	}
	
	static void brace_close(TokIt tok_it) {
		if (braces::braces.size() > 1) {
			int index = braces::braces_end_index(false);
			Brace open_brace = braces::get_last_open_index(index);
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
		message::error("Closing brace unexpected.");
	}
	
	static void quote(TokIt tok_it, int &str_index, bool &in_quote) {
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
			
			Type type{get_type_of_name("ch").value()};
			type.is_pointer = true;
			
			out.insert(DATA_ASM+2, str + ' ' + combine_toks(tok_it-2, tok_it+1) + std::string(1, '\n'));
			commit(replace_toks(_us_ltoks, (str == ".ascii" ? tok_it-3 : tok_it-2), tok_it, ".STR" + std::to_string(str_index) + "(%rip)"));
			variables.push_back(Variable{".STR" + std::to_string(str_index), type, -1, braces::braces_end_index()});
			str_index++;
		}
		in_quote = !in_quote;
	}
	
}

static int begin_compile(std::vector<std::string> args) {
	try {

	std::string output_dir = "./";
	if (args.size() < 2) {
		message::error("Must input file to compile.", 0);
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
		std::smatch chunk{get_innermost_parentheses(l)};
		bool no_parentheses{chunk.str().find('(') == std::string::npos};
		while (!chunk.empty()) {
			line_number++;
			std::string _chunk = trim(chunk.str());
			_ltoks = split(_chunk);
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
						message::error("Cannot declare function. Function of this name already exists.");
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
					if (get_type_of_name(*(tok_it-1)).has_value()) {
						message::error("Cannot define variable. Variable of this name already exists.");
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
			while_us_find_token("->", 1, 1, [&](TokIt tok_it) {
				token_function::cast(tok_it);
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
			while_us_find_tokens(type_names(), 0, 1, [&](TokIt tok_it) {
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

			l.erase(l.begin()+chunk.position(), l.begin()+chunk.position()+chunk.length());
			l.insert(chunk.position(), combine_toks(_us_ltoks.begin(), _us_ltoks.end()));
			
			if (no_parentheses) {
				break;
			} else {
				l.erase(l.begin()+chunk.position());
				size_t end = l.find(')', std::distance(l.begin(), l.begin()+chunk.position()));
				if (end == std::string::npos) {
					message::error("No matching ending parenthesis found.");
				}
				l.erase(l.begin()+end);
				chunk = get_innermost_parentheses(l);
				no_parentheses = (chunk.str().find('(') == std::string::npos);
			}
		}
		
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

