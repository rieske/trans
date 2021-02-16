#ifndef _TOKEN_H_
#define _TOKEN_H_

#include <string>

#include "translation_unit/Context.h"

class Token {
public:
    Token(std::string id, std::string lexeme, const translation_unit::Context& context);
    Token(const Token& that);

    const std::string id;
    const std::string lexeme;
    const translation_unit::Context context;
};

#endif // _TOKEN_H_
