#ifndef TYPES_TYPEQUERY_H_
#define TYPES_TYPEQUERY_H_

#include "Type.h"

#include <cstring>
#include <string>

namespace type {

// Use kind(), not isPrimitive(): pointer-to-T still carries T's payload.
inline bool isFloating(const Type& t) {
    return t.kind() == TypeKind::Primitive && t.getPrimitive().isFloating();
}

inline bool isIntegral(const Type& t) {
    return t.kind() == TypeKind::Primitive && !t.getPrimitive().isFloating();
}

// True when arithmetic / shifts should treat `t` as unsigned (pointers/arrays
// are address values; unsigned integrals; floats are not).
inline bool isUnsignedSide(const Type& t) {
    if (t.kind() == TypeKind::Pointer || t.kind() == TypeKind::Array) {
        return true;
    }
    if (isIntegral(t)) {
        return !t.getPrimitive().isSigned();
    }
    return false;
}

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

// Parse a C floating literal token into IEEE-754 bit patterns for codegen.
// Returns true and sets bitsOut on success.
inline bool floatingLiteralBits(const std::string& token, unsigned long long& bitsOut) {
    std::string digits = stripFloatSuffix(token);
    if (digits.empty()) {
        return false;
    }
    try {
        // Prefer double; float suffixes still produce a double constant then
        // assign narrows when the result type is float.
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

// Memory access width for loads/stores through pointers / struct fields.
// Natural size for packed 1/2/4-byte fields; otherwise a full qword.
inline int memoryAccessSizeBytes(const Type& t) {
    int size = t.getSize();
    if (size == 1 || size == 2 || size == 4) {
        return size;
    }
    return 8;
}

// Byte size for stack/global Value homes (empty types still get a word).
inline int valueSizeBytes(const Type& t) {
    int size = t.getSize();
    return size <= 0 ? 8 : size;
}

// Signedness for sub-word memory loads (sign-extend vs zero-extend).
// Only signed integral primitives (kind Primitive) sign-extend. Pointers still
// carry a pointee payload in Type's representation — use kind(), not isPrimitive().
inline bool memoryAccessIsSigned(const Type& t) {
    if (t.kind() != TypeKind::Primitive || t.getPrimitive().isFloating()) {
        return false;
    }
    return t.getPrimitive().isSigned();
}

// Signedness for live Values / stack homes (SAR default, EOF -1 comparisons).
// Signed integrals report their signedness; non-integrals (incl. pointers) default signed.
inline bool valueIsSigned(const Type& t) {
    if (t.kind() == TypeKind::Primitive && !t.getPrimitive().isFloating()) {
        return t.getPrimitive().isSigned();
    }
    return true;
}

// INTEGRAL vs FLOATING for codegen::Value construction.
inline bool valueIsFloating(const Type& t) {
    return isFloating(t);
}

// Integer promotions (C 6.3.1.1): types narrower than int convert to int.
inline Type integerPromote(const Type& t) {
    if (!isIntegral(t)) {
        return t;
    }
    if (t.getSize() > 0 && t.getSize() < 4) {
        return signedInteger();
    }
    return t;
}

} // namespace type

#endif // TYPES_TYPEQUERY_H_
