#ifndef _TOKEN_H_
#define _TOKEN_H_

#include <string>

class Token {
public:
	Token(int is, std::string lexeme);
	Token(const Token& that);

	int getId() const;
	std::string getLexeme() const;

	unsigned line;
private:
	int id;
	std::string lexeme;
};

#endif // _TOKEN_H_
