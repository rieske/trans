#include "StringLiteralExpression.h"
#include "ast/AbstractSyntaxTreeVisitor.h"
#include "symbols/AnnotationStore.h"
#include "util/StringLiteralDecode.h"

#include <utility>

namespace ast {

StringLiteralExpression::StringLiteralExpression(std::string value, translation_unit::Context context):
    value { std::move(value) },
    context {context}
{
    type::Type element = type::signedCharacter();
    const int unit = util::stringLiteralUnitBytes(this->value);
    if (unit == 2) {
        element = type::unsignedShort();
    } else if (unit == 4) {
        // char32_t is unsigned. wchar_t is signed.
        element = this->value[0] == 'U' ? type::unsignedInteger() : type::signedInteger();
    }
    setType(type::array(element, util::stringLiteralArrayLength(this->value)));
}

translation_unit::Context StringLiteralExpression::getContext() const {
    return context;
}

const std::string& StringLiteralExpression::getValue() const {
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

