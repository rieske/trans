#include "ast/IdentifierExpression.h"

#include "ast/AbstractSyntaxTreeVisitor.h"

namespace ast {

IdentifierExpression::IdentifierExpression(std::string identifier, translation_unit::Context context) :
        identifier { identifier },
        context { context }
{
    lval = true;
}

IdentifierExpression::~IdentifierExpression() {
}

void IdentifierExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

translation_unit::Context IdentifierExpression::getContext() const {
    return context;
}

std::string IdentifierExpression::getIdentifier() const {
    return identifier;
}

}
