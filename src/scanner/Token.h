#ifndef _TOKEN_H_
#define _TOKEN_H_

#include <string>

#include "TranslationUnitContext.h"

class Token {
public:
	Token(std::string id, std::string lexeme, const TranslationUnitContext& context);
	Token(const Token& that);

	const std::string id;
	const std::string lexeme;
	const TranslationUnitContext context;
};

#endif // _TOKEN_H_
