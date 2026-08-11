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

inline bool tokenHasFloatSuffix(const std::string& token) {
    if (token.empty()) {
        return false;
    }
    char c = token.back();
    return c == 'f' || c == 'F';
}

struct FloatingBits {
    unsigned long long bits { 0 };
    int sizeBytes { 8 };
};

inline unsigned long long float32ToBits(float value) {
    unsigned bits32 = 0;
    std::memcpy(&bits32, &value, sizeof(bits32));
    return bits32;
}

inline unsigned long long float64ToBits(double value) {
    unsigned long long bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}

inline float bitsToFloat32(unsigned long long bits) {
    auto bits32 = static_cast<unsigned>(bits);
    float value = 0;
    std::memcpy(&value, &bits32, sizeof(value));
    return value;
}

inline double bitsToFloat64(unsigned long long bits) {
    double value = 0;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

inline unsigned long long encodeFloating(double value, int sizeBytes) {
    if (sizeBytes == 4) {
        return float32ToBits(static_cast<float>(value));
    }
    return float64ToBits(value);
}

inline double decodeFloating(unsigned long long bits, int sizeBytes) {
    if (sizeBytes == 4) {
        return bitsToFloat32(bits);
    }
    return bitsToFloat64(bits);
}

// Parse a C floating literal into IEEE bits. f/F -> 32-bit pattern (size 4);
// otherwise 64-bit double (size 8).
inline bool floatingLiteralBits(const std::string& token, FloatingBits& out) {
    std::string digits = stripFloatSuffix(token);
    if (digits.empty()) {
        return false;
    }
    try {
        if (tokenHasFloatSuffix(token)) {
            float f = std::stof(digits);
            unsigned bits32 = 0;
            static_assert(sizeof(float) == sizeof(unsigned), "unexpected float size");
            std::memcpy(&bits32, &f, sizeof(bits32));
            out.bits = bits32;
            out.sizeBytes = 4;
            return true;
        }
        double d = std::stod(digits);
        static_assert(sizeof(double) == sizeof(unsigned long long), "unexpected double size");
        unsigned long long bits = 0;
        std::memcpy(&bits, &d, sizeof(bits));
        out.bits = bits;
        out.sizeBytes = 8;
        return true;
    } catch (...) {
        return false;
    }
}

// Parse token to assembler immediate, or return false if not a floating literal.
inline bool floatingLiteralImmediate(const std::string& token, std::string& immediateOut) {
    FloatingBits parsed;
    if (!floatingLiteralBits(token, parsed)) {
        return false;
    }
    immediateOut = hexImmediate(parsed.bits);
    return true;
}

} // namespace util

#endif // UTIL_FLOATINGLITERAL_H_
