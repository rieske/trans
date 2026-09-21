#include "ConstantExpression.h"

#include <utility>

#include "AbstractSyntaxTreeVisitor.h"
#include "types/IntegerConstant.h"
#include "util/IntegerLiteral.h"
#include "util/StringLiteralDecode.h"

namespace ast {

ConstantExpression::ConstantExpression(Constant constant) :
        constant { std::move(constant) }
{
    setType(this->constant.getType());
}

void ConstantExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

std::optional<type::Type> ConstantExpression::typeAtParseTime(const ParseEnvironment&) const {
    return expressionType();
}

translation_unit::Context ConstantExpression::getContext() const {
    return constant.getContext();
}

const std::string& ConstantExpression::getValue() const {
    return constant.getValue();
}

bool ConstantExpression::evaluateConstant(type::IntegerConstant& value) const {
    if (!hasExpressionType()) {
        return false;
    }
    const std::string& token = constant.getValue();
    long charValue = 0;
    if (util::decodeCharConstant(token, charValue)) {
        value = type::convert(type::fromHostLong(charValue), expressionType());
        return true;
    }
    util::IntegerLiteral lit;
    if (!util::parseIntegerLiteral(token, lit)) {
        return false;
    }
    value = type::fromLiteralBits(lit.value, expressionType());
    return true;
}

} // namespace ast

