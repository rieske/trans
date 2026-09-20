#ifndef _TOKEN_H_
#define _TOKEN_H_

#include <cstdint>
#include <string>
#include <string_view>

#include "translation_unit/Context.h"

namespace scanner {

enum class TokenClass : std::uint8_t {
    Lexeme,
    End,
    Id,
    TypedefName,
    String,
    IntConst,
    FloatConst,
    CharConst,
};

TokenClass tokenClassFromName(std::string_view className);

class Token {
public:
    Token(std::string className, std::string lexeme, const translation_unit::Context& context,
            int symbolId = -1);

    static const std::string END;

    TokenClass cls;
    std::string lexeme;
    translation_unit::Context context;
    int symbolId;
};

} // namespace scanner

#endif // _TOKEN_H_
