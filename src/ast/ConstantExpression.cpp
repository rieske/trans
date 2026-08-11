#include "ConstantExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "util/StringLiteralDecode.h"

namespace ast {

namespace {

// Integer literal or a character constant ('a', '\n', '\xFE', '\033').
bool parseConstantToken(const std::string& token, long& value) {
    if (util::decodeCharConstant(token, value)) {
        return true;
    }
    try {
        // base 0: honor C's 0-prefix octal and 0x hex, else decimal.
        value = std::stol(token, nullptr, 0);
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace

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

translation_unit::Context ConstantExpression::getContext() const {
    return constant.getContext();
}

std::string ConstantExpression::getValue() const {
    return constant.getValue();
}

bool ConstantExpression::evaluateConstant(long& value) const {
    return parseConstantToken(constant.getValue(), value);
}

} // namespace ast

