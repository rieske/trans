#ifndef _TOKEN_H_
#define _TOKEN_H_

#include <string>

#include "translation_unit/Context.h"

namespace scanner {

class Token {
public:
    Token(std::string id, std::string lexeme, const translation_unit::Context& context);
    Token(std::string id, std::string lexeme, const translation_unit::Context& context, int symbolId);

    static const std::string END;

    std::string id;
    std::string lexeme;
    translation_unit::Context context;
    int symbolId;
};

} // namespace scanner

#endif // _TOKEN_H_
