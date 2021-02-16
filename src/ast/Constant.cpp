#include "Constant.h"

#include <memory>

namespace ast {

Constant::Constant(std::string value, IntegralType type, translation_unit::Context context) :
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

std::unique_ptr<FundamentalType> Constant::getType() const {
    return std::unique_ptr<FundamentalType>{type.clone()};
}

} // namespace ast
