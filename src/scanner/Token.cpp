#include "Token.h"

Token::Token(std::string id, std::string lexeme, int line) :
		id { id },
		lexeme { lexeme },
		line { line } {
}

Token::Token(const Token& that) :
		id { that.id },
		lexeme { that.lexeme },
		line { that.line } {
}

std::string Token::getId() const {
	return id.empty() ? "'$end$'" : id;
}

std::string Token::getLexeme() const {
	return lexeme;
}
