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

// f/F -> 4, l/L -> 16, otherwise 8. f wins if both suffixes appear.
inline int floatingLiteralSizeBytes(const std::string& token) {
    bool isFloat = false;
    bool isLong = false;
    for (std::size_t i = token.size(); i > 0; --i) {
        const char c = token[i - 1];
        if (c == 'f' || c == 'F') {
            isFloat = true;
        } else if (c == 'l' || c == 'L') {
            isLong = true;
        } else {
            break;
        }
    }
    if (isFloat) {
        return 4;
    }
    if (isLong) {
        return 16;
    }
    return 8;
}

struct FloatingBits {
    unsigned long long bits { 0 };
    unsigned long long bitsHi { 0 };
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

inline FloatingBits encodeLongDouble(long double value) {
    FloatingBits out;
    unsigned char buf[16] = {};
    std::memcpy(buf, &value, 10);
    std::memcpy(&out.bits, buf, 8);
    std::memcpy(&out.bitsHi, buf + 8, 8);
    out.sizeBytes = 16;
    return out;
}

inline FloatingBits encodeFloatingBits(double value, int sizeBytes) {
    if (sizeBytes == 16) {
        return encodeLongDouble(static_cast<long double>(value));
    }
    FloatingBits out;
    out.bits = encodeFloating(value, sizeBytes);
    out.sizeBytes = sizeBytes;
    return out;
}

// IEEE 1.0 (float/double) or 80-bit x87 1.0 packed in 16 bytes.
inline FloatingBits floatingOne(int sizeBytes) {
    FloatingBits out;
    if (sizeBytes == 4) {
        out.bits = encodeFloating(1.0, 4);
        out.sizeBytes = 4;
        return out;
    }
    if (sizeBytes == 16) {
        out.bits = 0x8000000000000000ull;
        out.bitsHi = 0x3fffull;
        out.sizeBytes = 16;
        return out;
    }
    out.bits = encodeFloating(1.0, 8);
    out.sizeBytes = 8;
    return out;
}

inline double decodeFloating(unsigned long long bits, int sizeBytes) {
    if (sizeBytes == 4) {
        return bitsToFloat32(bits);
    }
    return bitsToFloat64(bits);
}

inline long double decodeLongDouble(const FloatingBits& bits) {
    unsigned char buf[16] = {};
    std::memcpy(buf, &bits.bits, 8);
    std::memcpy(buf + 8, &bits.bitsHi, 8);
    long double value = 0;
    std::memcpy(&value, buf, 10);
    return value;
}

inline long double decodeFloatingValue(const FloatingBits& bits) {
    if (bits.sizeBytes > 8) {
        return decodeLongDouble(bits);
    }
    return static_cast<long double>(decodeFloating(bits.bits, bits.sizeBytes));
}

inline double decodeFloatingBits(const FloatingBits& bits) {
    return static_cast<double>(decodeFloatingValue(bits));
}

inline void negateFloating(FloatingBits& bits) {
    if (bits.sizeBytes > 8) {
        bits.bitsHi ^= 0x8000ull;
        return;
    }
    bits.bits = encodeFloating(-decodeFloating(bits.bits, bits.sizeBytes), bits.sizeBytes);
}

// Parse a C floating literal into IEEE bits. f/F -> 32-bit (size 4);
// l/L -> 80-bit x87 in 16 bytes; otherwise 64-bit double (size 8).
inline bool floatingLiteralBits(const std::string& token, FloatingBits& out) {
    std::string digits = stripFloatSuffix(token);
    if (digits.empty()) {
        return false;
    }
    try {
        const int size = floatingLiteralSizeBytes(token);
        if (size == 4) {
            float f = std::stof(digits);
            unsigned bits32 = 0;
            static_assert(sizeof(float) == sizeof(unsigned), "unexpected float size");
            std::memcpy(&bits32, &f, sizeof(bits32));
            out.bits = bits32;
            out.bitsHi = 0;
            out.sizeBytes = 4;
            return true;
        }
        if (size == 16) {
            static_assert(sizeof(long double) >= 10, "host long double is not 80-bit");
            long double v = std::stold(digits);
            unsigned char buf[16] = {};
            std::memcpy(buf, &v, 10);
            unsigned long long lo = 0;
            unsigned long long hi = 0;
            std::memcpy(&lo, buf, 8);
            std::memcpy(&hi, buf + 8, 8);
            out.bits = lo;
            out.bitsHi = hi;
            out.sizeBytes = 16;
            return true;
        }
        double d = std::stod(digits);
        static_assert(sizeof(double) == sizeof(unsigned long long), "unexpected double size");
        unsigned long long bits = 0;
        std::memcpy(&bits, &d, sizeof(bits));
        out.bits = bits;
        out.bitsHi = 0;
        out.sizeBytes = 8;
        return true;
    } catch (...) {
        return false;
    }
}

// C floating lexeme (suffixes ok). False when the value does not fit in 64 bits.
inline bool floatingLiteralImmediate(const std::string& token, std::string& immediateOut) {
    FloatingBits parsed;
    if (!floatingLiteralBits(token, parsed) || parsed.sizeBytes > 8) {
        return false;
    }
    immediateOut = hexImmediate(parsed.bits);
    return true;
}

} // namespace util

#endif // UTIL_FLOATINGLITERAL_H_
