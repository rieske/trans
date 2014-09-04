#include "TypeSpecifier.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

TypeSpecifier::TypeSpecifier( BasicType type, std::string name) :
        name { name },
        type { type } {
}

TypeSpecifier::~TypeSpecifier() {
}

void TypeSpecifier::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

const std::string& TypeSpecifier::getName() const {
    return name;
}

BasicType TypeSpecifier::getType() const {
    return type;
}

} /* namespace ast */
