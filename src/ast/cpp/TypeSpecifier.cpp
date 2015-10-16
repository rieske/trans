#include "ast/TypeSpecifier.h"

namespace ast {

TypeSpecifier::TypeSpecifier(FundamentalType& type, std::string name) :
        name { name },
        type { std::unique_ptr<FundamentalType> { type.clone() } }
{
}

TypeSpecifier::TypeSpecifier(const TypeSpecifier& copyFrom) :
        name { copyFrom.name },
        type { std::unique_ptr<FundamentalType> { copyFrom.type->clone() } }
{
}

const std::string& TypeSpecifier::getName() const {
    return name;
}

std::unique_ptr<FundamentalType> TypeSpecifier::getType() const {
    return std::unique_ptr<FundamentalType> { type->clone() };
}

} /* namespace ast */
