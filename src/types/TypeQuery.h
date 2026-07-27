#ifndef TYPES_TYPEQUERY_H_
#define TYPES_TYPEQUERY_H_

#include <string>

#include "Type.h"

namespace type {

// Prefer these over raw isFunction()/isPointer() combinations.
// Recursive Type: pointer is its own kind; bare function is Function and not a pointer.

inline bool isBareFunction(const Type& t) {
    return t.isFunction();
}

// Non-floating primitive scalar (not a pointer — isPrimitive already excludes indirection).
inline bool isIntegralScalar(const Type& t) {
    return t.isPrimitive() && !t.getPrimitive().isFloating();
}

// Pointee size for pointer arithmetic / indexing (at least 1).
inline int pointerElementStride(const Type& ptrType) {
    int size = ptrType.dereference().getSize();
    return size > 0 ? size : 1;
}

// Closed classification of C pointer additive ops (shared by SA result typing and CG IR choice).
enum class PointerArithmeticForm {
    None,        // no pointer operand involved
    PtrPlusInt,  // ptr + int
    IntPlusPtr,  // int + ptr
    PtrMinusInt, // ptr - int
    PtrMinusPtr, // ptr - ptr
    Invalid,     // pointer involved but not a legal form
};

struct PointerArithmeticInfo {
    PointerArithmeticForm form { PointerArithmeticForm::None };
    Type resultType { voidType() };
    int strideBytes { 1 };
};

// Classify `left op right` for op in {+, -}. Result type is the pointer type or int (ptrdiff).
inline PointerArithmeticInfo classifyPointerArithmetic(const Type& left, const Type& right, char op) {
    PointerArithmeticInfo info;
    if (op != '+' && op != '-') {
        return info;
    }
    if (!left.isPointer() && !right.isPointer()) {
        return info;
    }
    if (op == '+' && left.isPointer() && isIntegralScalar(right)) {
        info.form = PointerArithmeticForm::PtrPlusInt;
        info.resultType = left;
        info.strideBytes = pointerElementStride(left);
        return info;
    }
    if (op == '+' && isIntegralScalar(left) && right.isPointer()) {
        info.form = PointerArithmeticForm::IntPlusPtr;
        info.resultType = right;
        info.strideBytes = pointerElementStride(right);
        return info;
    }
    if (op == '-' && left.isPointer() && isIntegralScalar(right)) {
        info.form = PointerArithmeticForm::PtrMinusInt;
        info.resultType = left;
        info.strideBytes = pointerElementStride(left);
        return info;
    }
    if (op == '-' && left.isPointer() && right.isPointer()) {
        info.form = PointerArithmeticForm::PtrMinusPtr;
        info.resultType = signedInteger();
        info.strideBytes = pointerElementStride(left);
        return info;
    }
    info.form = PointerArithmeticForm::Invalid;
    return info;
}

inline bool isPointerToFunction(const Type& t) {
    return t.isPointer() && t.dereference().isFunction();
}

// After recursive Type, pointer-to-function is just Pointer with Function pointee;
// this name is kept as an alias for existing call sites.
inline bool isPointerToBareFunction(const Type& t) {
    return isPointerToFunction(t);
}

inline bool isIncompleteObjectType(const Type& t) {
    return t.isVoid() || isBareFunction(t) || t.isIncompleteRecord();
}

// Void, bare function, or incomplete record (not pointer-to-incomplete).
inline bool isIncompleteMemberOrElementType(const Type& t) {
    return t.isVoid() || isBareFunction(t) || t.isIncompleteRecord();
}

inline bool isFloating(const Type& t) {
    return t.isPrimitive() && t.getPrimitive().isFloating();
}

inline bool isIntegral(const Type& t) {
    return t.isPrimitive() && !t.getPrimitive().isFloating();
}

inline bool isArithmeticType(const Type& t) {
    return isIntegral(t) || isFloating(t);
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

// Signedness for live Values / stack homes (SAR default).
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

// Usual arithmetic conversions (product subset): floating -> double;
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

// Primitive or pointer (caller must decay arrays/functions if desired).
inline bool isProductScalar(const Type& t) {
    return t.kind() == TypeKind::Primitive || t.isPointer();
}

// Operand compatibility after array/function decay (not assignment).
bool productValueCompatible(const Type& a, const Type& b);

// Git-shaped assign gate (assignment / init / call args). Not a pure subset of
// productValueCompatible: arrays never assign; incomplete dest rejected;
// function designators only into function-pointer dest; null-integer into pointers.
bool productAssignFrom(const Type& dest, const Type& source);

// Alias kept for existing call sites (same policy as productAssignFrom).
inline bool productCanAssignFrom(const Type& dest, const Type& source) {
    return productAssignFrom(dest, source);
}

// Scalar arithmetic (* / % and non-pointer +/-): both arithmetic types.
bool productArithmeticCompatible(const Type& a, const Type& b);

// Diagnostic text for a failed product assign (call only when canAssign is false).
std::string productAssignFailureMessage(const Type& dest, const Type& source);

// Array subscript element info for SA (shared policy).
struct ArraySubscriptInfo {
    Type elementType { voidType() };
    int elementStride { 8 };
    bool baseIsArray { false };

    bool valid() const { return elementStride > 0; }
};

// Byte size of one index step through a value of type t (at least 1).
// For array types this is the whole array size (e.g. sizeof(int[3]) for p where p is int(*)[3]).
inline int objectStrideBytes(const Type& t) {
    int size = t.getSize();
    return size < 1 ? 1 : size;
}

// Given the C type of the subscript base (array or pointer).
inline ArraySubscriptInfo arraySubscriptInfo(const Type& baseType) {
    ArraySubscriptInfo info;
    if (baseType.isArray()) {
        info.elementType = baseType.getElementType();
        // Index steps by sizeof(element), not sizeof(the whole array).
        info.elementStride = objectStrideBytes(info.elementType);
        info.baseIsArray = true;
    } else if (baseType.isPointer()) {
        info.elementType = baseType.dereference();
        // p is T(*)[N]: stride is sizeof(T[N]); otherwise sizeof(pointee).
        info.elementStride = objectStrideBytes(info.elementType);
        info.baseIsArray = false;
    } else {
        info.elementType = voidType();
        info.elementStride = 0;
        info.baseIsArray = false;
    }
    return info;
}

// Dual-type subscript: expression type may still be T[N] while value type is
// already a decayed pointer.
inline ArraySubscriptInfo arraySubscriptInfo(const Type& expressionType, const Type& valueType) {
    if (expressionType.isArray() && valueType.isPointer()) {
        ArraySubscriptInfo info;
        info.elementType = expressionType.getElementType();
        info.elementStride = objectStrideBytes(info.elementType);
        info.baseIsArray = false;
        return info;
    }
    ArraySubscriptInfo sub = arraySubscriptInfo(expressionType);
    if (!sub.valid() && valueType.isPointer()) {
        ArraySubscriptInfo info;
        info.elementType = valueType.dereference();
        info.elementStride = objectStrideBytes(info.elementType);
        info.baseIsArray = false;
        return info;
    }
    return sub;
}

} // namespace type

#endif // TYPES_TYPEQUERY_H_
