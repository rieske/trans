#include "Token.h"

namespace scanner {

Token::Token(std::string id, std::string lexeme, const translation_unit::Context& context) :
        id { id.empty() ? "'$end$'" : id },
        lexeme { lexeme },
        context { context }
{
}

Token::Token(const Token& that) :
        id { that.id },
        lexeme { that.lexeme },
        context { that.context }
{
}

} // namespace scanner

