#include "StringLiteralExpression.h"
#include "ast/AbstractSyntaxTreeVisitor.h"

namespace ast {

StringLiteralExpression::StringLiteralExpression(std::string value, translation_unit::Context context):
    value {value},
    context {context}
{}

StringLiteralExpression::~StringLiteralExpression() = default;

translation_unit::Context StringLiteralExpression::getContext() const {
    return context;
}

std::string StringLiteralExpression::getValue() const {
    return value;
}

void StringLiteralExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace ast

