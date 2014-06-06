#include "Token.h"

Token::Token(int id, std::string lexeme, int line) :
		id { id },
		lexeme { lexeme },
		line { line } {
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
