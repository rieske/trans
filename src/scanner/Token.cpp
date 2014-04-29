#include "Token.h"

Token::Token(int id, std::string lexeme) :
		id { id },
		lexeme { lexeme },
		line { 0 } {
}

Token::Token(const Token& that) :
		id { that.id },
		lexeme { that.lexeme },
		line { that.line } {
}

int Token::getId() const {
	return id;
}

std::string Token::getLexeme() const {
	return lexeme;
}
