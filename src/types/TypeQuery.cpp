#include "TypeQuery.h"

namespace type {

namespace {

Type productDecay(Type t) {
    if (t.isArray()) {
        return t.decayArray();
    }
    if (isBareFunction(t)) {
        return pointer(t);
    }
    return t;
}

bool recordsCompatible(const Type& a, const Type& b) {
    return a.isRecord() && b.isRecord();
}

// Product-loose null constant: integral (including null pointer constant 0).
bool isNullConstantCandidate(const Type& t) {
    return isIntegral(t);
}

} // namespace

bool productValueCompatible(const Type& a, const Type& b) {
    Type la = productDecay(a);
    Type ra = productDecay(b);
    if (la.isVoid() || ra.isVoid()) {
        return false;
    }
    // Product-loose record-to-record (struct/union); pin tests cover structure pairs
    // and union pairs intentionally.
    if (la.isRecord() || ra.isRecord()) {
        return recordsCompatible(la, ra);
    }
    return isProductScalar(la) && isProductScalar(ra);
}

bool productAssignFrom(const Type& dest, const Type& source) {
    // Own gate (not "valueCompatible plus ..."): see TypeQuery.h.
    if (dest.isVoid() || isBareFunction(dest)) {
        return false;
    }
    if (dest.isIncompleteRecord()) {
        return false;
    }
    if (dest.isArray() || source.isArray()) {
        return false;
    }

    if (isPointerToFunction(dest)) {
        if (isBareFunction(source) || isPointerToFunction(source)) {
            return true;
        }
        return isNullConstantCandidate(source);
    }
    if (dest.isPointer()) {
        if (source.isPointer()) {
            return true;
        }
        return isNullConstantCandidate(source);
    }
    if (isBareFunction(source) || isPointerToFunction(source)) {
        return false;
    }
    if (dest.isRecord() || source.isRecord()) {
        return recordsCompatible(dest, source);
    }
    return isProductScalar(dest) && isProductScalar(source);
}

bool productArithmeticCompatible(const Type& a, const Type& b) {
    return isArithmeticType(a) && isArithmeticType(b);
}

std::string productAssignFailureMessage(const Type& dest, const Type& source) {
    if ((isBareFunction(source) || isPointerToFunction(source)) && !isPointerToFunction(dest)) {
        return "function designator used as a value is not supported";
    }
    return "type mismatch: can't convert " + source.to_string() + " to " + dest.to_string();
}

} // namespace type
