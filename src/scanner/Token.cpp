#include "Token.h"

namespace scanner {

const std::string Token::END = "'$end$'";

Token::Token(std::string id, std::string lexeme, const translation_unit::Context& context) :
        Token { std::move(id), std::move(lexeme), context, -1 }
{
}

Token::Token(std::string id, std::string lexeme, const translation_unit::Context& context, int symbolId) :
        id { id.empty() ? END : id },
        lexeme { lexeme },
        context { context },
        symbolId { symbolId }
{
}

} // namespace scanner

