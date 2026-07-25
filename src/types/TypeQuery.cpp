#include "TypeQuery.h"

namespace type {

namespace {

bool isProductScalar(const Type& t) {
    return t.isPrimitive() || t.isPointer();
}

} // namespace

bool productCanAssignFrom(const Type& dest, const Type& source) {
    if (dest.isVoid() || isBareFunction(dest)) {
        return false;
    }
    if (dest.isIncompleteStructure()) {
        return false;
    }
    if (dest.isArray() || source.isArray()) {
        return false;
    }

    // Function-pointer destination: designator, pointer-to-function, or integer
    // (null pointer constant 0 — product does not require constant-expression proof).
    if (isPointerToFunction(dest)) {
        if (isBareFunction(source) || isPointerToFunction(source)) {
            return true;
        }
        return source.isPrimitive() && !source.getPrimitive().isFloating();
    }
    // Other pointers: accept integer null constant or compatible pointers.
    if (dest.isPointer()) {
        if (source.isPointer()) {
            return true;
        }
        return source.isPrimitive() && !source.getPrimitive().isFloating();
    }
    // Function-pointer / designator source into non-fp: reject.
    if (isBareFunction(source) || isPointerToFunction(source)) {
        return false;
    }

    // Structures: only structure-to-structure (not pointer address temps).
    if (dest.isStructure()) {
        return source.isStructure();
    }
    if (source.isStructure()) {
        return false;
    }

    // Scalars and non-function pointers.
    if (isProductScalar(dest) && isProductScalar(source)) {
        return true;
    }

    return false;
}

std::string productAssignFailureMessage(const Type& dest, const Type& source) {
    // Same predicates as productCanAssignFrom — presentation only.
    if ((isBareFunction(source) || isPointerToFunction(source)) && !isPointerToFunction(dest)) {
        return "function designator used as a value is not supported";
    }
    return "type mismatch: can't convert " + source.to_string() + " to " + dest.to_string();
}

} // namespace type
