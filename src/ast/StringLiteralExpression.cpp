#include "StringLiteralExpression.h"
#include "ast/AbstractSyntaxTreeVisitor.h"

namespace ast {

StringLiteralExpression::StringLiteralExpression(std::string value, translation_unit::Context context):
    value {value},
    context {context}
{
    setType(type::pointer(type::signedCharacter(), {type::Qualifier::CONST}));
}

StringLiteralExpression::~StringLiteralExpression() = default;

translation_unit::Context StringLiteralExpression::getContext() const {
    return context;
}

std::string StringLiteralExpression::getValue() const {
    return value;
}

void StringLiteralExpression::setConstantSymbol(std::string constantSymbol) {
    this->constantSymbol = constantSymbol;
}

std::string StringLiteralExpression::getConstantSymbol() const {
    return constantSymbol;
}

void StringLiteralExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace ast

