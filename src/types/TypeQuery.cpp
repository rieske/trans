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

// Type-only null constant: integral 0. ((void*)0) is handled in SA with a source expression.
bool isNullConstantCandidate(const Type& t) {
    return isIntegral(t);
}

} // namespace

bool productAssignFrom(const Type& dest, const Type& source) {
    // Own gate (not "valueCompatible plus ..."): see TypeQuery.h.
    if (dest.isVoid() || isBareFunction(dest)) {
        return false;
    }
    if (dest.isIncompleteRecord()) {
        return false;
    }
    if (dest.isArray()) {
        return false;
    }
    const Type src = productDecay(source);

    if (dest.isTransparentUnion()) {
        if (isNullConstantCandidate(src)) {
            return true;
        }
        for (const auto& member : dest.getMembers()) {
            if (member.type && productAssignFrom(*member.type, src)) {
                return true;
            }
        }
        return false;
    }

    if (isPointerToFunction(dest)) {
        if (isBareFunction(src) || isPointerToFunction(src)) {
            return true;
        }
        // Integer 0 only at type-only gate; (void*)0 / NULL need sourceExpr in SA.
        return isNullConstantCandidate(src);
    }
    if (dest.isPointer()) {
        if (src.isPointer()) {
            return true;
        }
        return isNullConstantCandidate(src);
    }
    if (isBareFunction(src) || isPointerToFunction(src)) {
        return false;
    }
    if (dest.isRecord() || src.isRecord()) {
        return recordsCompatible(dest, src);
    }
    return isProductScalar(dest) && isProductScalar(src);
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
