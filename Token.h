#include <string>
#include <vector>

class Token {
public:
	Token(std::string token, bool word = false) : token(token) { }
	Token() : token("") { }

	std::string token;
};

static std::vector<Token> tokens = {
	Token(" "), Token("#>"), Token("??>"), Token("?>"), Token("#"), Token("("), Token(")"),
	Token("\""), Token("//"), Token("*"), Token("/"), Token("%"), Token("+"), Token("-"),
	Token("<="), Token(">="), Token(">"), Token("<"), Token("^^"), Token("^"), Token("&"),
	Token("=="), Token("!="), Token(">"), Token("<"), Token("="), Token("??"), Token("?")
};

