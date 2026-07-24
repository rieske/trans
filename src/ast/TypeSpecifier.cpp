#include "TypeSpecifier.h"

namespace ast {

TypeSpecifier::TypeSpecifier(type::Type type, std::string name) :
        name { name },
        type { type }
{
}

TypeSpecifier::TypeSpecifier(type::Type type, std::string name, std::vector<std::pair<std::string, long>> enumerators) :
        name { name },
        type { type },
        enumerators { std::move(enumerators) }
{
}

const std::string& TypeSpecifier::getName() const {
    return name;
}

type::Type TypeSpecifier::getType() const {
    return type;
}

const std::vector<std::pair<std::string, long>>& TypeSpecifier::getEnumerators() const {
    return enumerators;
}

bool TypeSpecifier::hasEnumerators() const {
    return !enumerators.empty();
}

} // namespace ast
