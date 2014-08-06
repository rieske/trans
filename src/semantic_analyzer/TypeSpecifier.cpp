#include "TypeSpecifier.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

TypeSpecifier::TypeSpecifier( BasicType type, std::string name) :
        name { name },
        type { type } {
}

TypeSpecifier::~TypeSpecifier() {
}

void TypeSpecifier::accept(AbstractSyntaxTreeVisitor& visitor) const {
    visitor.visit(*this);
}

const std::string& TypeSpecifier::getName() const {
    return name;
}

BasicType TypeSpecifier::getType() const {
    return type;
}

} /* namespace semantic_analyzer */
