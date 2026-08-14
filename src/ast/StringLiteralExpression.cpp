#include "StringLiteralExpression.h"
#include "ast/AbstractSyntaxTreeVisitor.h"
#include "util/StringLiteralDecode.h"

namespace ast {

StringLiteralExpression::StringLiteralExpression(std::string value, translation_unit::Context context):
    value {value},
    context {context}
{
    setType(type::array(type::signedCharacter(), util::stringLiteralArrayLength(value)));
    lval = true;
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

std::optional<type::Type> StringLiteralExpression::typeAtParseTime(const ParseEnvironment&) const {
    return expressionType();
}

void StringLiteralExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace ast

