#ifndef _TOKEN_H_
#define _TOKEN_H_

#include <string>

class Token {
public:
	Token(std::string id, std::string lexeme, int line);
	Token(const Token& that);

	std::string getId() const;
	std::string getLexeme() const;

	int line;
private:
	std::string id;
	std::string lexeme;
};

#endif // _TOKEN_H_
