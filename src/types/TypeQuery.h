#ifndef TYPES_TYPEQUERY_H_
#define TYPES_TYPEQUERY_H_

#include "Type.h"

#include <cstring>
#include <string>

namespace type {

// Prefer these over raw payload inspection (Type is a recursive sum of kinds).
inline bool isFloating(const Type& t) {
    return t.isPrimitive() && t.getPrimitive().isFloating();
}

inline bool isIntegral(const Type& t) {
    return t.isPrimitive() && !t.getPrimitive().isFloating();
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
// Only signed integral primitives sign-extend.
inline bool memoryAccessIsSigned(const Type& t) {
    if (!isIntegral(t)) {
        return false;
    }
    return t.getPrimitive().isSigned();
}

// Signedness for live Values / stack homes (SAR default, EOF -1 comparisons).
// Signed integrals report their signedness; non-integrals (incl. pointers) default signed.
inline bool valueIsSigned(const Type& t) {
    if (isIntegral(t)) {
        return t.getPrimitive().isSigned();
    }
    return true;
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

// Usual arithmetic conversions (C 6.3.1.8, product subset): floating -> double;
// otherwise integer promotions then wider (and unsigned-over-signed) wins.
inline Type usualArithmeticResult(const Type& left, const Type& right) {
    if (isFloating(left) || isFloating(right)) {
        return doubleFloating();
    }
    Type leftP = integerPromote(left);
    Type rightP = integerPromote(right);
    if (rightP.getSize() > leftP.getSize()) {
        return rightP;
    }
    if (rightP.getSize() == leftP.getSize()
            && isIntegral(rightP) && isIntegral(leftP)
            && !valueIsSigned(rightP) && valueIsSigned(leftP)) {
        return rightP;
    }
    return leftP;
}

// Product-loose scalar (primitive or pointer) after array/function decay.
inline bool isProductScalar(const Type& t) {
    return t.kind() == TypeKind::Primitive || t.isPointer();
}

// Product-loose assignability (git-shaped C): dest may accept source.
// Sole implementation in TypeQuery.cpp — not ISO can_assign. See ARCHITECTURE.md.
bool productCanAssignFrom(const Type& dest, const Type& source);

// Array subscript element info for SA (shared policy, not mid-visitor specials).
struct ArraySubscriptInfo {
    Type elementType { voidType() };
    int elementStride { 8 };
    bool baseIsArray { false };
};

// Given the C type of the subscript base (array or pointer), return element type,
// stride for indexing, and whether the base object is an array (vs pointer value).
inline ArraySubscriptInfo arraySubscriptInfo(const Type& baseType) {
    ArraySubscriptInfo info;
    if (baseType.isArray()) {
        info.elementType = baseType.getElementType();
        info.elementStride = baseType.getElementStride();
        info.baseIsArray = true;
    } else if (baseType.isPointer()) {
        info.elementType = baseType.dereference();
        info.elementStride = info.elementType.getSize();
        if (info.elementStride < 1) {
            info.elementStride = 1;
        }
        if (info.elementType.isArray()) {
            // p is T(*)[N]: stride is sizeof(T[N]) for p[i].
            info.elementStride = info.elementType.getSize();
        }
        info.baseIsArray = false;
    } else {
        info.elementType = voidType();
        info.elementStride = 0;
        info.baseIsArray = false;
    }
    return info;
}


// C 6.4.5 / 6.5.3.4: sizeof "abc" is the array size including NUL, not sizeof(char*).
// The AST types string literals as const char* for decay; sizeof must use the
// lexical length. `token` is the scanner string token (quotes included).
// Implemented via util::stringLiteralArrayLength — declared here for SA policy.
int sizeofStringLiteralTokenBytes(const std::string& token);

} // namespace type

#endif // TYPES_TYPEQUERY_H_
