#ifndef _TOKEN_H_
#define _TOKEN_H_

#include <string>
#include <string_view>

#include "translation_unit/Context.h"

namespace scanner {

class Token {
public:
    Token(std::string_view id, std::string_view lexeme, const translation_unit::Context& context,
            int symbolId = -1);

    static const std::string END;

    std::string id;
    std::string lexeme;
    translation_unit::Context context;
    int symbolId;
};

} // namespace scanner

#endif // _TOKEN_H_
