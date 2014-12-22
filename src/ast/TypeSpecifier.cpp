#include "TypeSpecifier.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "types/BaseType.h"

namespace ast {

TypeSpecifier::TypeSpecifier(std::unique_ptr<BaseType> type, std::string name) :
        name { name },
        type { std::move(type) }
{
}

TypeSpecifier::TypeSpecifier(const TypeSpecifier& copyFrom) :
        name { copyFrom.name },
        type { copyFrom.type->clone() }
{
}

TypeSpecifier::~TypeSpecifier() {
}

void TypeSpecifier::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

const std::string& TypeSpecifier::getName() const {
    return name;
}

std::unique_ptr<BaseType> TypeSpecifier::getType() const {
    return type->clone();
}

} /* namespace ast */
