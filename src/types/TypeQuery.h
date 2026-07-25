#ifndef TYPES_TYPEQUERY_H_
#define TYPES_TYPEQUERY_H_

#include <string>

#include "Type.h"

namespace type {

// Prefer these over raw isFunction()/isPointer() combinations.
// Pointer-to-function also reports isFunction() because pointer() copies the function payload.

inline bool isBareFunction(const Type& t) {
    return t.isFunction() && !t.isPointer();
}

inline bool isPointerToFunction(const Type& t) {
    return t.isPointer() && t.dereference().isFunction();
}

inline bool isPointerToBareFunction(const Type& t) {
    return t.isPointer() && isBareFunction(t.dereference());
}

inline bool isIncompleteObjectType(const Type& t) {
    return t.isVoid() || isBareFunction(t) || t.isIncompleteStructure();
}

inline bool isIncompleteMemberOrElementType(const Type& t) {
    return t.isVoid() || isBareFunction(t);
}

bool productCanAssignFrom(const Type& dest, const Type& source);

// Diagnostic text for a failed productCanAssignFrom (call only when canAssign is false).
std::string productAssignFailureMessage(const Type& dest, const Type& source);

// Array subscript element info for SA (shared policy — finish-for-git TypeQuery).
struct ArraySubscriptInfo {
    Type elementType { voidType() };
    int elementStride { 8 };
    bool baseIsArray { false };

    bool valid() const { return elementStride > 0; }
};

// Given the C type of the subscript base (array or pointer).
inline ArraySubscriptInfo arraySubscriptInfo(const Type& baseType) {
    ArraySubscriptInfo info;
    if (baseType.isArray()) {
        info.elementType = baseType.getElementType();
        info.elementStride = info.elementType.getSize();
        if (info.elementStride < 1) {
            info.elementStride = 1;
        }
        info.baseIsArray = true;
    } else if (baseType.isPointer()) {
        info.elementType = baseType.dereference();
        info.elementStride = info.elementType.getSize();
        if (info.elementStride < 1) {
            info.elementStride = 1;
        }
        info.baseIsArray = false;
    } else {
        info.elementType = voidType();
        info.elementStride = 0;
        info.baseIsArray = false;
    }
    return info;
}

// Dual-type: expression type may still be T[N] while value is a decayed pointer
// (multi-dim row / array-of-struct address). Prefer expression type for element/stride.
inline ArraySubscriptInfo arraySubscriptInfo(const Type& expressionType, const Type& valueType) {
    if (expressionType.isArray() && valueType.isPointer()) {
        ArraySubscriptInfo info;
        info.elementType = expressionType.getElementType();
        info.elementStride = info.elementType.getSize();
        if (info.elementStride < 1) {
            info.elementStride = 1;
        }
        info.baseIsArray = false;
        return info;
    }
    ArraySubscriptInfo sub = arraySubscriptInfo(expressionType);
    if (!sub.valid() && valueType.isPointer()) {
        ArraySubscriptInfo info;
        info.elementType = valueType.dereference();
        info.elementStride = info.elementType.getSize();
        if (info.elementStride < 1) {
            info.elementStride = 1;
        }
        info.baseIsArray = false;
        return info;
    }
    return sub;
}

} // namespace type

#endif // TYPES_TYPEQUERY_H_
