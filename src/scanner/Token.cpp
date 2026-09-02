#include "Token.h"

namespace scanner {

const std::string Token::END = "'$end$'";

Token::Token(std::string_view id, std::string_view lexeme, const translation_unit::Context& context, int symbolId) :
        id { id.empty() ? END : std::string { id } },
        lexeme { lexeme },
        context { context },
        symbolId { symbolId }
{
}

} // namespace scanner

