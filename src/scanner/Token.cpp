#include "Token.h"

#include <iostream>

Token::Token(std::string id, std::string lexeme, int line) :
		id { id.empty() ? "'$end$'" : id},
		lexeme { lexeme },
		line { line } {
}

Token::Token(const Token& that) :
		id { that.id },
		lexeme { that.lexeme },
		line { that.line } {
}
