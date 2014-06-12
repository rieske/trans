#ifndef _TOKEN_H_
#define _TOKEN_H_

#include <string>

class Token {
public:
	Token(std::string id, std::string lexeme, int line);
	Token(const Token& that);

	const std::string id;
	const std::string lexeme;
	const int line;
};

#endif // _TOKEN_H_
