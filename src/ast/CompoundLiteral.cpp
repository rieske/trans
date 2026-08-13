#include "CompoundLiteral.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

CompoundLiteral::CompoundLiteral(TypeSpecifier typeSpecifier,
        std::unique_ptr<InitializerListExpression> initializer) :
        typeSpecifier { std::move(typeSpecifier) },
        initializer_ { std::move(initializer) } {
    lval = true;
}

void CompoundLiteral::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

translation_unit::Context CompoundLiteral::getContext() const {
    return initializer_->getContext();
}

TypeSpecifier& CompoundLiteral::getTypeSpecifier() {
    return typeSpecifier;
}

const TypeSpecifier& CompoundLiteral::getTypeSpecifier() const {
    return typeSpecifier;
}

InitializerListExpression& CompoundLiteral::initializer() {
    return *initializer_;
}

const InitializerListExpression& CompoundLiteral::initializer() const {
    return *initializer_;
}

} // namespace ast
