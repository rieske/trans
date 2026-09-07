#include "Token.h"

#include <utility>

namespace scanner {

const std::string Token::END = "'$end$'";

Token::Token(std::string id, std::string lexeme, const translation_unit::Context& context, int symbolId) :
        id { id.empty() ? END : std::move(id) },
        lexeme { std::move(lexeme) },
        context { context },
        symbolId { symbolId }
{
}

} // namespace scanner

