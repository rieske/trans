#include "Token.h"

namespace scanner {

std::string Token::END = "'$end$'";

Token::Token(std::string id, std::string lexeme, const translation_unit::Context& context) :
        id { id.empty() ? END : id },
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

