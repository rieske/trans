#ifndef UTIL_FLOATINGLITERAL_H_
#define UTIL_FLOATINGLITERAL_H_

#include "ImmediateFormat.h"

#include <cstring>
#include <string>

namespace util {

// Strip float/double literal suffixes (f/F/l/L) for stod / bit conversion.
inline std::string stripFloatSuffix(std::string digits) {
    while (!digits.empty()) {
        char c = digits.back();
        if (c == 'f' || c == 'F' || c == 'l' || c == 'L') {
            digits.pop_back();
        } else {
            break;
        }
    }
    return digits;
}

// Parse a C floating literal token into IEEE-754 double bits for codegen.
// Float suffixes still produce double bits; assign narrows when the result is float.
inline bool floatingLiteralBits(const std::string& token, unsigned long long& bitsOut) {
    std::string digits = stripFloatSuffix(token);
    if (digits.empty()) {
        return false;
    }
    try {
        double d = std::stod(digits);
        static_assert(sizeof(double) == sizeof(unsigned long long), "unexpected double size");
        unsigned long long bits = 0;
        std::memcpy(&bits, &d, sizeof(bits));
        bitsOut = bits;
        return true;
    } catch (...) {
        return false;
    }
}

// Parse token to assembler immediate, or return false if not a floating literal.
inline bool floatingLiteralImmediate(const std::string& token, std::string& immediateOut) {
    unsigned long long bits = 0;
    if (!floatingLiteralBits(token, bits)) {
        return false;
    }
    immediateOut = hexImmediate(bits);
    return true;
}

} // namespace util

#endif // UTIL_FLOATINGLITERAL_H_
