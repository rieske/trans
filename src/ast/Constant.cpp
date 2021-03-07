#include "Constant.h"

#include <memory>

namespace ast {

Constant::Constant(std::string value, type::Type type, translation_unit::Context context) :
        value { value },
        type { type },
        context { context }
{
}

translation_unit::Context Constant::getContext() const {
    return context;
}

std::string Constant::getValue() const {
    return value;
}

type::Type Constant::getType() const {
    return type;
}

} // namespace ast

