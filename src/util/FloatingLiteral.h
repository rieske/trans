#ifndef UTIL_FLOATINGLITERAL_H_
#define UTIL_FLOATINGLITERAL_H_

#include "FloatingBits.h"
#include "ImmediateFormat.h"

#include <cstring>
#include <string>

namespace util {

inline bool isFloatTypeSuffixChar(char c) {
    return c == 'f' || c == 'F' || c == 'l' || c == 'L';
}

inline bool isImaginarySuffixChar(char c) {
    return c == 'i' || c == 'I';
}

inline bool hasImaginarySuffix(const std::string& token) {
    for (std::size_t i = token.size(); i > 0; --i) {
        const char c = token[i - 1];
        if (isImaginarySuffixChar(c)) {
            return true;
        }
        if (!isFloatTypeSuffixChar(c)) {
            break;
        }
    }
    return false;
}

// Strip float/double literal suffixes (f/F/l/L) and GNU imaginary i/I.
inline std::string stripFloatSuffix(std::string digits) {
    while (!digits.empty()) {
        char c = digits.back();
        if (isFloatTypeSuffixChar(c) || isImaginarySuffixChar(c)) {
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
        if (isImaginarySuffixChar(c)) {
            continue;
        }
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

inline FloatingBits encodeLongDouble(long double value) {
    FloatingBits out;
    out.sizeBytes = 16;
    unsigned long long words[2] = { 0, 0 };
    // 80-bit payload; do not copy the 6 padding bytes (unspecified).
    std::memcpy(words, &value, 10);
    out.bits = words[0];
    out.bitsHi = words[1];
    return out;
}

inline long double bitsToLongDouble(const FloatingBits& fp) {
    unsigned long long words[2] = { fp.bits, fp.bitsHi };
    long double value = 0;
    std::memcpy(&value, words, 10);
    return value;
}

inline FloatingBits encodeFloating(long double value, int sizeBytes) {
    FloatingBits out;
    out.sizeBytes = sizeBytes;
    if (sizeBytes == 4) {
        out.bits = float32ToBits(static_cast<float>(value));
        return out;
    }
    if (sizeBytes == 16) {
        return encodeLongDouble(value);
    }
    out.sizeBytes = 8;
    out.bits = float64ToBits(static_cast<double>(value));
    return out;
}

inline FloatingBits floatingOne(int sizeBytes) {
    return encodeFloating(1.0L, sizeBytes);
}

inline long double decodeFloating(const FloatingBits& fp) {
    if (fp.sizeBytes == 4) {
        return bitsToFloat32(fp.bits);
    }
    if (fp.sizeBytes == 16) {
        return bitsToLongDouble(fp);
    }
    return bitsToFloat64(fp.bits);
}

inline FloatingBits convertFloating(const FloatingBits& src, int destSize) {
    if (src.sizeBytes == destSize) {
        return src;
    }
    return encodeFloating(decodeFloating(src), destSize);
}

// Flip the IEEE sign bit without rounding through a narrower host type.
inline FloatingBits negateFloating(FloatingBits fp) {
    if (fp.sizeBytes == 4) {
        fp.bits ^= 0x80000000ull;
    } else if (fp.sizeBytes == 16) {
        // 80-bit sign is bit 15 of the exponent word.
        fp.bitsHi ^= 0x8000ull;
    } else {
        fp.bits ^= 0x8000000000000000ull;
    }
    return fp;
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
            out = encodeFloating(std::stof(digits), 4);
            return true;
        }
        if (size == 16) {
            out = encodeLongDouble(std::stold(digits));
            return true;
        }
        out = encodeFloating(std::stod(digits), 8);
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
