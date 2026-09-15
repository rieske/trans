#include "Constant.h"

#include <utility>

namespace ast {

Constant::Constant(std::string value, type::Type type, translation_unit::Context context) :
        value { std::move(value) },
        type { std::move(type) },
        context { context }
{
}

translation_unit::Context Constant::getContext() const {
    return context;
}

const std::string& Constant::getValue() const {
    return value;
}

const type::Type& Constant::getType() const {
    return type;
}

} // namespace ast

