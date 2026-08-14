#include "ConstantExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "util/IntegerLiteral.h"
#include "util/StringLiteralDecode.h"

namespace ast {

namespace {

// Integer literal or a character constant ('a', '\n', '\xFE', '\033').
bool parseConstantToken(const std::string& token, long& value) {
    if (util::decodeCharConstant(token, value)) {
        return true;
    }
    util::IntegerLiteral lit;
    if (!util::parseIntegerLiteral(token, lit) || lit.value > ~0ull) {
        return false;
    }
    value = static_cast<long>(static_cast<unsigned long long>(lit.value));
    return true;
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

std::optional<type::Type> ConstantExpression::typeAtParseTime(const ParseEnvironment&) const {
    return getType();
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

