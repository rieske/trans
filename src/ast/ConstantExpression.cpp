#include "ConstantExpression.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

namespace {

// Decode an integer literal or a single-character constant ('a', '\n') to its value.
bool parseConstantToken(const std::string& token, long& value) {
    if (token.size() >= 3 && token.front() == '\'' && token.back() == '\'') {
        const std::string inner = token.substr(1, token.size() - 2);
        if (inner.size() == 1) {
            value = static_cast<unsigned char>(inner[0]);
            return true;
        }
        if (inner.size() == 2 && inner[0] == '\\') {
            switch (inner[1]) {
                case 'n': value = '\n'; return true;
                case 't': value = '\t'; return true;
                case 'r': value = '\r'; return true;
                case '0': value = '\0'; return true;
                case '\\': value = '\\'; return true;
                case '\'': value = '\''; return true;
                case '"': value = '"'; return true;
            }
        }
        return false;
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

