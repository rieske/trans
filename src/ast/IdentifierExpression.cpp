#include "IdentifierExpression.h"

#include "AbstractSyntaxTreeVisitor.h"

#include <cassert>

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

void IdentifierExpression::clearFoldedConstant() {
    foldedConstant.reset();
    lval = true;
}

bool IdentifierExpression::hasFoldedConstant() const {
    return foldedConstant.has_value();
}

long IdentifierExpression::getFoldedConstant() const {
    assert(foldedConstant.has_value());
    return *foldedConstant;
}

bool IdentifierExpression::evaluateConstant(long& value) const {
    if (foldedConstant) {
        value = *foldedConstant;
        return true;
    }
    return false;
}

void IdentifierExpression::setStringConstantLabel(std::string label) {
    stringConstantLabel = std::move(label);
    lval = false;
}

bool IdentifierExpression::hasStringConstantLabel() const {
    return stringConstantLabel.has_value();
}

const std::string& IdentifierExpression::getStringConstantLabel() const {
    assert(stringConstantLabel.has_value());
    return *stringConstantLabel;
}

} // namespace ast
