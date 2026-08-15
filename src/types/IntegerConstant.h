#ifndef TYPES_INTEGERCONSTANT_H_
#define TYPES_INTEGERCONSTANT_H_

#include <optional>
#include <string>

#include "Type.h"

namespace type {

// Integer ICE: object bits, canonical for `type`.
__extension__ typedef unsigned __int128 Bits;
__extension__ typedef __int128 SignedBits;

struct IntegerConstant {

    Bits bits { 0 };
    Type type;

    IntegerConstant() :
            type { signedInteger() } {
    }

    IntegerConstant(Bits bits, Type type) :
            bits { bits },
            type { std::move(type) } {
    }
};

// C 6.4.4.1: first type in the suffix/base list that can represent `value`.
IntegerConstant rankedLiteral(Bits value, int base, bool uns, bool lng);

IntegerConstant convert(const IntegerConstant& src, const Type& dest);
IntegerConstant fromHostLong(long value);
IntegerConstant fromLiteralBits(Bits bits, const Type& dest);
long toHostLong(const IntegerConstant& value);
bool isZero(const IntegerConstant& value);

inline SignedBits signedValue(const IntegerConstant& value) {
    return static_cast<SignedBits>(value.bits);
}

// Smallest of {int, unsigned, long, unsigned long, i128, u128} covering [min, max].
Type enumUnderlyingType(SignedBits min, SignedBits max);
IntegerConstant nextEnumerator(const IntegerConstant& value);

inline unsigned long long bitsWord(const IntegerConstant& value, int index) {
    return static_cast<unsigned long long>(value.bits >> (index * 64));
}

std::optional<IntegerConstant> foldUnary(const std::string& op, IntegerConstant operand);
std::optional<IntegerConstant> foldBinary(const std::string& op, IntegerConstant left,
        IntegerConstant right);

} // namespace type

#endif
