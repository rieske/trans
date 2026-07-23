#include "IdentifierExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "scanner/EnumConstantRegistry.h"

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
    // Parse-time enum folding: later enumerators may reference earlier ones
    // (e.g. _SC_IOV_MAX = _SC_UIO_MAXIOV in system headers).
    return scanner::EnumConstantRegistry::lookup(identifier, value);
}

void IdentifierExpression::setFunctionDesignator(std::string name) {
    functionDesignator = std::move(name);
    lval = false;
}

bool IdentifierExpression::hasFunctionDesignator() const {
    return functionDesignator.has_value();
}

const std::string& IdentifierExpression::getFunctionDesignator() const {
    return *functionDesignator;
}

} // namespace ast
