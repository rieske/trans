#ifndef _TOKEN_H_
#define _TOKEN_H_

#include <string>

class Token {
public:
	unsigned type;
	std::string value;

	unsigned line;
};

#endif // _TOKEN_H_
