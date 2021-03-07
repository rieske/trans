#include "TypeSpecifier.h"

namespace ast {

TypeSpecifier::TypeSpecifier(type::Type type, std::string name) :
        name { name },
        type { type }
{
}

const std::string& TypeSpecifier::getName() const {
    return name;
}

type::Type TypeSpecifier::getType() const {
    return type;
}

} // namespace ast

