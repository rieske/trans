#ifndef _TOKEN_H_
#define _TOKEN_H_

#include <string>

class Token {
public:
	Token(int type, std::string value);
	Token(const Token& that);

	int type;
	std::string value;

	unsigned line;
};

#endif // _TOKEN_H_
