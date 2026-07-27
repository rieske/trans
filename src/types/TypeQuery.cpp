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

} // namespace

bool productValueCompatible(const Type& a, const Type& b) {
    Type la = productDecay(a);
    Type ra = productDecay(b);
    if (la.isVoid() || ra.isVoid()) {
        return false;
    }
    // Product-loose record-to-record (master policy; spike tightens by body identity later).
    if (la.isRecord() || ra.isRecord()) {
        return la.isRecord() && ra.isRecord();
    }
    if (isProductScalar(la) && isProductScalar(ra)) {
        return true;
    }
    return false;
}

bool productAssignFrom(const Type& dest, const Type& source) {
    // Product assign (git-shaped). Keep master rejections that functional/unit tests pin:
    // - no bare-function destination
    // - no incomplete record destination
    // - no array assign
    // - no function designator / fp into non-fp destination
    if (dest.isVoid() || isBareFunction(dest)) {
        return false;
    }
    if (dest.isIncompleteStructure()) {
        return false;
    }
    if (dest.isArray() || source.isArray()) {
        return false;
    }

    if (isPointerToFunction(dest)) {
        if (isBareFunction(source) || isPointerToFunction(source)) {
            return true;
        }
        return source.isPrimitive() && !source.getPrimitive().isFloating();
    }
    if (dest.isPointer()) {
        if (source.isPointer()) {
            return true;
        }
        return source.isPrimitive() && !source.getPrimitive().isFloating();
    }
    if (isBareFunction(source) || isPointerToFunction(source)) {
        return false;
    }
    if (dest.isRecord()) {
        return source.isRecord();
    }
    if (source.isRecord()) {
        return false;
    }
    return isProductScalar(dest) && isProductScalar(source);
}

bool productArithmeticCompatible(const Type& a, const Type& b) {
    Type la = productDecay(a);
    Type ra = productDecay(b);
    if (la.isPointer() || ra.isPointer() || la.isRecord() || ra.isRecord()) {
        return false;
    }
    if (la.isVoid() || ra.isVoid()) {
        return false;
    }
    return (isIntegral(la) || isFloating(la)) && (isIntegral(ra) || isFloating(ra));
}

std::string productAssignFailureMessage(const Type& dest, const Type& source) {
    if ((isBareFunction(source) || isPointerToFunction(source)) && !isPointerToFunction(dest)) {
        return "function designator used as a value is not supported";
    }
    return "type mismatch: can't convert " + source.to_string() + " to " + dest.to_string();
}

} // namespace type
