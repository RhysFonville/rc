#include <string>
#include <type_traits>
#include <vector>

class Token {
public:
	Token(std::string token, bool word = false) : token(token)/*, is_word(is_wordii)*/ { }
	Token() : token("") { }

	std::string token;
};

static std::vector<Token> tokens = {
	Token(" "), Token("("), Token(")"), Token("\""), Token("*"), Token("/"), Token("%"), Token("+"),
	Token("-"), Token(">"), Token("<"), Token("^"), Token("=")
};

