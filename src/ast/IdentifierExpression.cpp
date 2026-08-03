#include "IdentifierExpression.h"

#include "AbstractSyntaxTreeVisitor.h"

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

void IdentifierExpression::setFoldedConstant(long value) {
    foldedConstant = value;
    // Enumerators are rvalues.
    lval = false;
}

bool IdentifierExpression::hasFoldedConstant() const {
    return foldedConstant.has_value();
}

long IdentifierExpression::getFoldedConstant() const {
    return *foldedConstant;
}

bool IdentifierExpression::evaluateConstant(long& value) const {
    if (foldedConstant) {
        value = *foldedConstant;
        return true;
    }
    // Enum constants must be folded at AST build (CSNB) via ParseEnvironment.
    return false;
}

} // namespace ast
