#include "ConstantExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "util/StringLiteralDecode.h"

namespace ast {

namespace {

// Decode an integer literal or a single-character constant ('a', '\n', '\033') to its value.
bool parseConstantToken(const std::string& token, long& value) {
    if (token.size() >= 3 && token.front() == '\'' && token.back() == '\'') {
        return util::decodeCharConstant(token, value);
    }
    try {
        // Strip C integer/float suffixes (u, l, ul, ull, f, F) before parsing.
        // Do not treat trailing f/F of a hex literal (0xff) as a float suffix.
        std::string digits = token;
        const bool looksHex = digits.size() >= 2 && digits[0] == '0'
                && (digits[1] == 'x' || digits[1] == 'X');
        while (!digits.empty()) {
            char c = digits.back();
            if (c == 'u' || c == 'U' || c == 'l' || c == 'L') {
                digits.pop_back();
            } else if (!looksHex && (c == 'f' || c == 'F')) {
                digits.pop_back();
            } else {
                break;
            }
        }
        if (digits.empty()) {
            return false;
        }
        // Floating literal: fold by truncating toward zero (enough for array sizes / cases).
        if (digits.find('.') != std::string::npos || digits.find('e') != std::string::npos
                || digits.find('E') != std::string::npos) {
            value = static_cast<long>(std::stod(digits));
            return true;
        }
        // base 0: honor C's 0-prefix octal and 0x hex, else decimal.
        // Prefer unsigned parse so values above LONG_MAX still fold.
        try {
            value = static_cast<long>(std::stoull(digits, nullptr, 0));
            return true;
        } catch (...) {
            value = std::stol(digits, nullptr, 0);
            return true;
        }
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

