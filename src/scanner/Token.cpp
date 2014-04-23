#include "Token.h"

Token::Token(int type, std::string value) :
		type { type },
		value { value },
		line{0} {
}

Token::Token(const Token& that) :
		type { that.type },
		value { that.value },
		line { that.line } {
}

