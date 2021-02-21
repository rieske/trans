#ifndef AST_STRING_LITERAL_H_
#define AST_STRING_LITERAL_H_

#include <string>

#include "translation_unit/Context.h"
#include "types/IntegralType.h"

namespace ast {

class StringLiteral {
public:
    StringLiteral(std::string value, translation_unit::Context context);
    virtual ~StringLiteral();

    translation_unit::Context getContext() const;
    std::string getValue() const;

private:
    std::string value;
    translation_unit::Context context;
};

} // namespace ast

#endif // AST_STRING_LITERAL_H_
