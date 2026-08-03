#include "TypeSpecifier.h"

#include "Expression.h"

namespace ast {

TypeSpecifier::TypeSpecifier(type::Type type, std::string name) :
        name { std::move(name) },
        type { std::move(type) }
{
}

TypeSpecifier::TypeSpecifier(type::Type type, std::string name,
        std::shared_ptr<Expression> typeofOperand) :
        name { std::move(name) },
        type { std::move(type) },
        typeofOperand { std::move(typeofOperand) }
{
}

TypeSpecifier TypeSpecifier::makeTypeof(std::shared_ptr<Expression> operand) {
    return TypeSpecifier { type::voidType(), "__typeof__", std::move(operand) };
}

const std::string& TypeSpecifier::getName() const {
    return name;
}

type::Type TypeSpecifier::getType() const {
    return type;
}

void TypeSpecifier::setResolvedType(type::Type resolved) {
    type = std::move(resolved);
    typeofOperand.reset(); // no longer provisional; getType() is the real type
}

} // namespace ast
