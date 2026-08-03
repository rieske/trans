#include "ConstantExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "types/IntegerConstant.h"
#include "util/IntegerLiteral.h"
#include "util/StringLiteralDecode.h"

namespace ast {

ConstantExpression::ConstantExpression(Constant constant) :
        constant { constant }
{
    setType(constant.getType());
}

ConstantExpression::~ConstantExpression() {
}

void ConstantExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

std::optional<type::Type> ConstantExpression::typeAtParseTime(const ParseEnvironment&) const {
    return constant.getType();
}

translation_unit::Context ConstantExpression::getContext() const {
    return constant.getContext();
}

std::string ConstantExpression::getValue() const {
    return constant.getValue();
}

bool ConstantExpression::evaluateConstant(type::IntegerConstant& value) const {
    if (!hasExpressionType()) {
        return false;
    }
    const std::string& token = constant.getValue();
    long charValue = 0;
    if (util::decodeCharConstant(token, charValue)) {
        value = type::convert(type::fromHostLong(charValue), getType());
        return true;
    }
    util::IntegerLiteral lit;
    if (!util::parseIntegerLiteral(token, lit)) {
        return false;
    }
    value = type::fromLiteralBits(lit.value, getType());
    return true;
}

} // namespace ast

