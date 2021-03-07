#ifndef AST_CONSTANT_H_
#define AST_CONSTANT_H_

#include <string>

#include "translation_unit/Context.h"
#include "types/Type.h"

namespace ast {

class Constant {
public:
    Constant(std::string value, type::Type type, translation_unit::Context context);
    virtual ~Constant() = default;

    translation_unit::Context getContext() const;
    std::string getValue() const;
    type::Type getType() const;

private:
    std::string value;
    type::Type type;
    translation_unit::Context context;
};

} // namespace ast

#endif // AST_CONSTANT_H_
