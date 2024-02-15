#include <vector>
#include <string>
#include <algorithm>
#include <ranges>
#include <iostream>
#include "Token.h"

using TokIt = std::vector<std::string>::iterator;

inline const char* const ws = " \t\n\r\f\v";

inline std::vector<std::string> _ltoks{};
inline std::vector<std::string> _us_ltoks{};
inline size_t line_number{0u};

inline std::vector<std::string> lines = { };

namespace clog {
	inline void out(const std::string &str) noexcept {
		std::cout << str << std::endl;
	}
	inline void warn(const std::string &str, bool print_line = true) noexcept {
		std::cerr << (print_line ? "LINE: " + std::to_string(line_number) : "") << "WARNING: " << str << std::endl;
	}
	inline void error(const std::string &str, bool print_line = true) {
		throw ((print_line ? "LINE: " + std::to_string(line_number) : "") + " ERROR: " + str + "\n\t" + lines[line_number]);
	}
	inline void note(const std::string &str) noexcept {
		std::cout << "NOTE: " << str << std::endl;
	}
}

template <typename T>
inline size_t from_it(const std::vector<T> &vec, const typename std::vector<T>::const_iterator &it) {
	return std::distance(vec.begin(), it);
}

inline std::vector<std::string>::const_iterator find_tok(const std::vector<std::string> &toks, const std::string &str,
				const std::vector<std::string>::const_iterator &begin) {
	return std::find(begin, toks.end(), str);
}

inline std::vector<std::string>::iterator find_tok(std::vector<std::string> &toks, const std::string &str,
				std::vector<std::string>::iterator &begin) {
	return std::find(begin, toks.end(), str);
}

inline std::vector<std::string>::iterator find_first_tok(std::vector<std::string> &toks, const std::vector<std::string> &toks_to_find,
				std::vector<std::string>::iterator &begin) {
	for (auto it = begin; it != toks.end(); it++) {
		for (const std::string &tok : toks_to_find) {
			if (*it == tok) return it;
		}
	}
	return toks.end();
}

template <typename Container, typename ConstIterator>
inline typename Container::iterator remove_constness(Container& c, ConstIterator it) {
    return c.erase(it, it);
}

inline void while_find_token(const std::string &tok, int begin, int end, const std::function<void(TokIt)> &func) {
	TokIt tok_it = _ltoks.begin();
	while (tok_it < _ltoks.end()) {
		tok_it = find_tok(_ltoks, tok, tok_it);
		if (tok_it != _ltoks.end()) {
			if (tok_it-begin < _ltoks.begin() || tok_it+end >= _ltoks.end()) clog::error("Token expected around \"" + *tok_it + "\", but there is none.");
			func(tok_it);
		} else {
			break;
		}
		tok_it++;
	}
}

inline void while_us_find_token(const std::string &tok, int begin, int end, const std::function<void(TokIt)> &func) {
	TokIt tok_it = _us_ltoks.begin();
	while (tok_it < _us_ltoks.end()) {
		tok_it = find_tok(_us_ltoks, tok, tok_it);
		if (tok_it != _us_ltoks.end()) {
			if (tok_it-begin < _us_ltoks.begin() || tok_it+end >= _us_ltoks.end()) clog::error("Token expected around \"" + *tok_it + "\", but there is none.");
			func(tok_it);
		} else {
			break;
		}
		tok_it++;
	}
}

inline void while_find_tokens(const std::vector<std::string> &toks, int begin, int end, const std::function<void(TokIt)> &func) {
	TokIt tok_it = _ltoks.begin();
	while (tok_it < _ltoks.end()) {
		tok_it = find_first_tok(_ltoks, toks, tok_it);
		if (tok_it != _ltoks.end()) {
			if (tok_it-begin < _ltoks.begin() || tok_it+end >= _ltoks.end()) clog::error("Token expected around \"" + *tok_it + "\", but there is none.");
			func(tok_it);
		} else {
			break;
		}
		tok_it++;
	}
}

inline void while_us_find_tokens(const std::vector<std::string> &toks, int begin, int end, const std::function<void(TokIt)> &func) {
	TokIt tok_it = _us_ltoks.begin();
	while (tok_it < _us_ltoks.end()) {
		tok_it = find_first_tok(_us_ltoks, toks, tok_it);
		if (tok_it != _us_ltoks.end()) {
			if (tok_it-begin < _us_ltoks.begin() || tok_it+end >= _us_ltoks.end()) clog::error("Token expected around \"" + *tok_it + "\", but there is none.");
			func(tok_it);
		} else {
			break;
		}
		tok_it++;
	}
}

// trim from end of string (right)
inline std::string rtrim(std::string s, const char* t = ws) {
	s.erase(s.find_last_not_of(t) + 1);
	return s;
}

// trim from beginning of string (left)
inline std::string ltrim(std::string s, const char* t = ws) {
	s.erase(0, s.find_first_not_of(t));
	return s;
}

// trim from both ends of string (right then left)
inline std::string trim(std::string s, const char* t = ws) {
	if (!s.empty())
		return ltrim(rtrim(s, t), t);
	else
		return "";
}

inline std::vector<std::string> replace_toks(std::vector<std::string> toks, size_t begin, size_t end, const std::string &str) {
	toks.erase(toks.begin()+begin, toks.begin()+end+1);
	toks.insert(toks.begin()+begin, str);
	
	return toks;
}

inline std::vector<std::string> replace_toks(const std::vector<std::string> &toks, std::vector<std::string>::const_iterator begin,
		std::vector<std::string>::const_iterator end, const std::string &str) {
	return replace_toks(toks, from_it(toks, begin), from_it(toks, end), str);
}

inline std::vector<std::string> replace_tok(std::vector<std::string> toks, size_t i, const std::string &str) {
	toks[i] = str;
	return toks;
}

inline std::vector<std::string> replace_tok(const std::vector<std::string> &toks, std::vector<std::string>::const_iterator it, const std::string &str) {
	return replace_tok(toks, from_it(toks, it), str);
}

inline std::string combine_toks(const std::vector<std::string>::const_iterator &begin, const std::vector<std::string>::const_iterator &end) {
	std::string ret = "";
	for (auto it = begin; it != end; it++) {
		ret += *it;
	}
	return ret;
}

inline std::vector<std::string> split(const std::string &str) { // IT WORKS!! WOW!!
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

inline std::vector<std::string> unspaced(const std::vector<std::string> &vec) {
	std::vector<std::string> ret;
	for (const std::string &str : vec) {
		if (str != " ") ret.push_back(str);
	}
	return ret;
}

template <typename T>
inline size_t index_of(const std::vector<T> &vec, const T &val) {
	return from_it(vec, std::find(vec.begin(), vec.end(), val));
}

template <typename T>
inline size_t index_of_last(const std::vector<T> &vec, const T &val) {
	return from_it(vec, std::find(vec.rbegin(), vec.rend(), val).base());
}


inline void commit(const std::vector<std::string> &new_vec) {
	_ltoks = new_vec;
	if (find_tok(new_vec, " ", new_vec.begin()) ==	new_vec.end()) {
		_us_ltoks = new_vec;
	} else {
		_us_ltoks = unspaced(_ltoks);
	}
}

