#include "IdentifierExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "ParseEnvironment.h"
#include "types/IntegerConstant.h"

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

std::optional<type::Type> IdentifierExpression::typeAtParseTime(const ParseEnvironment& environment) const {
    return environment.lookupValueType(identifier);
}

translation_unit::Context IdentifierExpression::getContext() const {
    return context;
}

std::string IdentifierExpression::getIdentifier() const {
    return identifier;
}

void IdentifierExpression::setFoldedConstant(type::IntegerConstant value) {
    foldedConstant = std::move(value);
    lval = false;
}

void IdentifierExpression::clearFoldedConstant() {
    foldedConstant.reset();
    lval = true;
}

bool IdentifierExpression::evaluateConstant(type::IntegerConstant& value) const {
    if (!foldedConstant) {
        return false;
    }
    value = *foldedConstant;
    return true;
}

void IdentifierExpression::setRodataLabel(symbols::AnnotationStore& store, std::string label) {
    store.setRodataLabel(this, std::move(label));
    lval = false;
}

const std::string* IdentifierExpression::rodataLabel(const symbols::AnnotationStore& store) const {
    return store.rodataLabel(this);
}

} // namespace ast
