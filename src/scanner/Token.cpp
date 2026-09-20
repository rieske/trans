#include "Token.h"

#include <utility>

namespace scanner {

const std::string Token::END = "'$end$'";

TokenClass tokenClassFromName(std::string_view id) {
    if (id.empty() || id == Token::END) {
        return TokenClass::End;
    }
    if (id == "id") {
        return TokenClass::Id;
    }
    if (id == "typedef_name") {
        return TokenClass::TypedefName;
    }
    if (id == "string") {
        return TokenClass::String;
    }
    if (id == "int_const") {
        return TokenClass::IntConst;
    }
    if (id == "float_const") {
        return TokenClass::FloatConst;
    }
    if (id == "char_const") {
        return TokenClass::CharConst;
    }
    return TokenClass::Lexeme;
}

Token::Token(std::string className, std::string lexeme, const translation_unit::Context& context, int symbolId) :
        cls { tokenClassFromName(className.empty() ? std::string_view { END } : std::string_view { className }) },
        lexeme { std::move(lexeme) },
        context { context },
        symbolId { symbolId }
{
}

} // namespace scanner
