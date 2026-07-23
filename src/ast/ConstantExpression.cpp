#include "ConstantExpression.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

namespace {

// Decode an integer literal or a single-character constant ('a', '\n', '\033') to its value.
bool parseConstantToken(const std::string& token, long& value) {
    if (token.size() >= 3 && token.front() == '\'' && token.back() == '\'') {
        const std::string inner = token.substr(1, token.size() - 2);
        if (inner.size() == 1) {
            value = static_cast<unsigned char>(inner[0]);
            return true;
        }
        if (inner.size() >= 2 && inner[0] == '\\') {
            // Simple one-letter escapes: \n \t \r \0 \\ \' \" \a \b \f \v
            if (inner.size() == 2) {
                switch (inner[1]) {
                    case 'n': value = '\n'; return true;
                    case 't': value = '\t'; return true;
                    case 'r': value = '\r'; return true;
                    case '0': value = '\0'; return true;
                    case 'a': value = '\a'; return true;
                    case 'b': value = '\b'; return true;
                    case 'f': value = '\f'; return true;
                    case 'v': value = '\v'; return true;
                    case '\\': value = '\\'; return true;
                    case '\'': value = '\''; return true;
                    case '"': value = '"'; return true;
                }
            }
            // Hex escapes: \xNN (one or more hex digits).
            if (inner.size() >= 3 && (inner[1] == 'x' || inner[1] == 'X')) {
                try {
                    value = static_cast<long>(std::stoul(inner.substr(2), nullptr, 16));
                    return true;
                } catch (...) {
                    return false;
                }
            }
            // Octal escapes: \0 .. \377 (one to three octal digits).
            if (inner[1] >= '0' && inner[1] <= '7') {
                try {
                    value = static_cast<long>(std::stoul(inner.substr(1), nullptr, 8));
                    return true;
                } catch (...) {
                    return false;
                }
            }
        }
        return false;
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

