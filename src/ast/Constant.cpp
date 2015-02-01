#include "Constant.h"

#include <memory>

#include "types/Type.h"

namespace ast {

Constant::Constant(std::string value, NumericType type, translation_unit::Context context) :
        value { value },
        type { type },
        context { context }
{
}

Constant::~Constant() {
}

translation_unit::Context Constant::getContext() const {
    return context;
}

std::string Constant::getValue() const {
    return value;
}

Type Constant::getType() const {
    return {type.clone()};
}

} /* namespace ast */
