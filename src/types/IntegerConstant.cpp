#include "IntegerConstant.h"

#include "TypeQuery.h"

#include <climits>
#include <vector>

namespace type {
namespace {

int widthBits(const Type& t) {
    return t.getSize() * 8;
}

Bits maskBits(int n) {
    if (n >= 128) {
        return ~Bits(0);
    }
    return (Bits(1) << n) - 1;
}

bool signedIntegral(const Type& t) {
    return isIntegral(t) && valueIsSigned(t);
}

Bits asUnsigned(const IntegerConstant& c) {
    const int n = widthBits(c.type);
    if (n > 0 && n < 128) {
        return c.bits & maskBits(n);
    }
    return c.bits;
}

bool fitsNonNeg(Bits value, const Type& t) {
    const int n = widthBits(t);
    if (!signedIntegral(t)) {
        return n >= 128 || value <= maskBits(n);
    }
    if (n >= 128) {
        return value <= (Bits(1) << 127) - 1;
    }
    return value <= (Bits(1) << (n - 1)) - 1;
}

IntegerConstant canonicalize(Bits bits, Type dest) {
    const int n = widthBits(dest);
    if (n <= 0 || n >= 128) {
        return { bits, std::move(dest) };
    }
    bits &= maskBits(n);
    if (signedIntegral(dest) && (bits & (Bits(1) << (n - 1)))) {
        bits |= ~maskBits(n);
    }
    return { bits, std::move(dest) };
}

} // namespace

IntegerConstant rankedLiteral(Bits value, int base, bool uns, bool lng) {
    const bool hexOrOct = base == 8 || base == 16;
    std::vector<Type> list;
    list.reserve(6);
    if (lng && uns) {
        list.push_back(unsignedLong());
        list.push_back(unsignedInt128());
    } else if (lng) {
        list.push_back(signedLong());
        if (hexOrOct) {
            list.push_back(unsignedLong());
        }
        list.push_back(signedInt128());
        if (hexOrOct) {
            list.push_back(unsignedInt128());
        }
    } else if (uns) {
        list.push_back(unsignedInteger());
        list.push_back(unsignedLong());
        list.push_back(unsignedInt128());
    } else if (hexOrOct) {
        list.push_back(signedInteger());
        list.push_back(unsignedInteger());
        list.push_back(signedLong());
        list.push_back(unsignedLong());
        list.push_back(signedInt128());
        list.push_back(unsignedInt128());
    } else {
        list.push_back(signedInteger());
        list.push_back(signedLong());
        list.push_back(signedInt128());
    }
    for (const Type& t : list) {
        if (fitsNonNeg(value, t)) {
            return canonicalize(value, t);
        }
    }
    return canonicalize(value, list.back());
}

IntegerConstant convert(const IntegerConstant& src, const Type& dest) {
    if (isBoolean(dest)) {
        return { !isZero(src), dest };
    }
    if (dest.isPointer()) {
        return { convert(src, unsignedLong()).bits, dest };
    }
    if (!isIntegral(dest)) {
        return src;
    }
    return canonicalize(src.bits, dest);
}

IntegerConstant fromHostLong(long value) {
    return canonicalize(static_cast<Bits>(static_cast<SignedBits>(value)), signedLong());
}

IntegerConstant fromLiteralBits(Bits bits, const Type& dest) {
    return canonicalize(bits, dest);
}

long toHostLong(const IntegerConstant& value) {
    return static_cast<long>(signedValue(convert(value, signedLong())));
}

bool isZero(const IntegerConstant& value) {
    return asUnsigned(value) == 0;
}

Type enumUnderlyingType(SignedBits min, SignedBits max) {
    if (min >= INT_MIN && max <= INT_MAX) {
        return signedInteger();
    }
    if (min >= 0 && max <= static_cast<SignedBits>(UINT_MAX)) {
        return unsignedInteger();
    }
    if (min >= LONG_MIN && max <= LONG_MAX) {
        return signedLong();
    }
    if (min >= 0 && max <= static_cast<SignedBits>(ULONG_MAX)) {
        return unsignedLong();
    }
    if (min >= 0) {
        return unsignedInt128();
    }
    return signedInt128();
}

IntegerConstant nextEnumerator(const IntegerConstant& value) {
    const Bits bits = value.bits + 1;
    const SignedBits v = static_cast<SignedBits>(bits);
    return fromLiteralBits(bits, enumUnderlyingType(v, v));
}

std::optional<IntegerConstant> foldUnary(const std::string& op, IntegerConstant operand) {
    if (op == "!") {
        return canonicalize(isZero(operand), signedInteger());
    }
    if (!isIntegral(operand.type)) {
        return std::nullopt;
    }
    operand = convert(operand, integerPromote(operand.type));
    if (op == "+") {
        return operand;
    }
    if (op == "-") {
        return canonicalize(static_cast<Bits>(-signedValue(operand)), operand.type);
    }
    if (op == "~") {
        return canonicalize(~operand.bits, operand.type);
    }
    return std::nullopt;
}

std::optional<IntegerConstant> foldBinary(const std::string& op, IntegerConstant left,
        IntegerConstant right) {
    if (op == "<<" || op == ">>") {
        if (!isIntegral(left.type) || !isIntegral(right.type)) {
            return std::nullopt;
        }
        left = convert(left, integerPromote(left.type));
        right = convert(right, integerPromote(right.type));
        const int width = widthBits(left.type);
        if (width <= 0 || (signedIntegral(right.type) && signedValue(right) < 0)
                || asUnsigned(right) >= static_cast<Bits>(width)) {
            return std::nullopt;
        }
        const int count = static_cast<int>(asUnsigned(right));
        if (op == "<<") {
            return canonicalize(left.bits << count, left.type);
        }
        if (signedIntegral(left.type)) {
            return canonicalize(static_cast<Bits>(signedValue(left) >> count), left.type);
        }
        return canonicalize(asUnsigned(left) >> count, left.type);
    }
    if (op == "&&") {
        return canonicalize(!isZero(left) && !isZero(right), signedInteger());
    }
    if (op == "||") {
        return canonicalize(!isZero(left) || !isZero(right), signedInteger());
    }
    if (!isIntegral(left.type) || !isIntegral(right.type)) {
        return std::nullopt;
    }
    const Type common = usualArithmeticResult(left.type, right.type);
    left = convert(left, common);
    right = convert(right, common);
    const bool uns = !signedIntegral(common);

    auto cmp = [&](bool signedLess, bool unsignedLess, bool eq) -> IntegerConstant {
        bool result = false;
        if (op == "<") {
            result = uns ? unsignedLess : signedLess;
        } else if (op == ">") {
            result = uns ? !unsignedLess && !eq : !signedLess && !eq;
        } else if (op == "<=") {
            result = uns ? unsignedLess || eq : signedLess || eq;
        } else if (op == ">=") {
            result = uns ? !unsignedLess : !signedLess;
        } else if (op == "==") {
            result = eq;
        } else if (op == "!=") {
            result = !eq;
        }
        return canonicalize(result, signedInteger());
    };

    if (op == "<" || op == ">" || op == "<=" || op == ">=" || op == "==" || op == "!=") {
        const bool eq = asUnsigned(left) == asUnsigned(right);
        return cmp(signedValue(left) < signedValue(right), asUnsigned(left) < asUnsigned(right), eq);
    }

    Bits result = 0;
    if (op == "+") {
        result = left.bits + right.bits;
    } else if (op == "-") {
        result = left.bits - right.bits;
    } else if (op == "*") {
        result = left.bits * right.bits;
    } else if (op == "/" || op == "%") {
        if (isZero(right)) {
            return std::nullopt;
        }
        if (uns) {
            result = op == "/" ? asUnsigned(left) / asUnsigned(right)
                               : asUnsigned(left) % asUnsigned(right);
        } else {
            result = op == "/" ? static_cast<Bits>(signedValue(left) / signedValue(right))
                               : static_cast<Bits>(signedValue(left) % signedValue(right));
        }
    } else if (op == "&") {
        result = left.bits & right.bits;
    } else if (op == "|") {
        result = left.bits | right.bits;
    } else if (op == "^") {
        result = left.bits ^ right.bits;
    } else {
        return std::nullopt;
    }
    return canonicalize(result, common);
}

} // namespace type
