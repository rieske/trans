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

void StringLiteralExpression::setRodataLabel(symbols::AnnotationStore& store, std::string label) {
    store.setRodataLabel(this, std::move(label));
}

const std::string* StringLiteralExpression::rodataLabel(const symbols::AnnotationStore& store) const {
    return store.rodataLabel(this);
}

std::optional<type::Type> StringLiteralExpression::typeAtParseTime(const ParseEnvironment&) const {
    return expressionType();
}

void StringLiteralExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace ast

