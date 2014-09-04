#include "Token.h"

#include <iostream>

Token::Token(std::string id, std::string lexeme, const TranslationUnitContext& context) :
		id { id.empty() ? "'$end$'" : id},
		lexeme { lexeme },
		context { context } {
}

Token::Token(const Token& that) :
		id { that.id },
		lexeme { that.lexeme },
		context { that.context } {
}
