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

// Non-floating, non-complex primitive scalar (not a pointer — isPrimitive already excludes indirection).
inline bool isIntegralScalar(const Type& t) {
    return t.isPrimitive() && !t.getPrimitive().isFloating() && !t.getPrimitive().isComplex();
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

// Void, bare function, incomplete record, or incomplete array (not pointer-to-incomplete).
// Shared definition used by sizeof and member/element completeness checks.
inline bool isIncompleteObjectType(const Type& t) {
    return t.isVoid() || isBareFunction(t) || t.isIncompleteRecord() || t.isIncompleteArray();
}

// Same predicate as isIncompleteObjectType; name documents member/element sites.
inline bool isIncompleteMemberOrElementType(const Type& t) {
    return isIncompleteObjectType(t);
}

inline bool isFloating(const Type& t) {
    return t.isPrimitive() && t.getPrimitive().isFloating();
}

inline bool isFloat(const Type& t) {
    return t.isPrimitive() && t.getPrimitive().kind() == PrimitiveKind::Float;
}

inline bool isDouble(const Type& t) {
    return t.isPrimitive() && t.getPrimitive().kind() == PrimitiveKind::Double;
}

inline bool isLongDouble(const Type& t) {
    return t.isPrimitive() && t.getPrimitive().kind() == PrimitiveKind::LongDouble;
}

inline bool isComplex(const Type& t) {
    return t.isPrimitive() && t.getPrimitive().isComplex();
}

inline bool isComplexFloat(const Type& t) {
    return t.isPrimitive() && t.getPrimitive().kind() == PrimitiveKind::ComplexFloat;
}

inline bool isComplexDouble(const Type& t) {
    return t.isPrimitive() && t.getPrimitive().kind() == PrimitiveKind::ComplexDouble;
}

inline bool isComplexLongDouble(const Type& t) {
    return t.isPrimitive() && t.getPrimitive().kind() == PrimitiveKind::ComplexLongDouble;
}

// Corresponding real type of a complex type; other types unchanged.
inline Type correspondingReal(const Type& t) {
    if (isComplexFloat(t)) {
        return floating();
    }
    if (isComplexDouble(t)) {
        return doubleFloating();
    }
    if (isComplexLongDouble(t)) {
        return longDoubleFloating();
    }
    return t;
}

// Complex type whose corresponding real type is `real` (float/double/long double).
inline Type complexOfReal(const Type& real) {
    if (isLongDouble(real)) {
        return complexLongDouble();
    }
    if (isDouble(real)) {
        return complexDouble();
    }
    return complexFloat();
}

inline bool isIntegral(const Type& t) {
    return isIntegralScalar(t);
}

inline bool isBoolean(const Type& t) {
    return t.isPrimitive() && t.getPrimitive().isBoolean();
}

inline bool isCharacter(const Type& t) {
    return t.isPrimitive() && t.getPrimitive().isCharacter();
}

// ISO real type: integer or real floating (not complex).
inline bool isRealType(const Type& t) {
    return isIntegral(t) || isFloating(t);
}

inline bool isArithmeticType(const Type& t) {
    return isRealType(t) || isComplex(t);
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

// Lvalue conversion (C 6.3.2.1): decay array/function, drop top-level cv.
inline Type afterLvalueConversion(const Type& t) {
    Type converted = t;
    if (converted.isArray()) {
        converted = converted.decayArray();
    } else if (converted.isFunction()) {
        converted = pointer(converted);
    }
    return converted.withoutTopLevelQualifiers();
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

// C 6.5.2.2: integer promotions, then float -> double. Other types unchanged.
inline Type defaultArgPromote(const Type& t) {
    if (isFloat(t)) {
        return doubleFloating();
    }
    return integerPromote(t);
}

// Assignment RHS convert dest. <<= >>=: integer-promote the count, not the LHS type.
inline Type assignmentConvertTarget(const std::string& op, const Type& dest, const Type& source) {
    if (op == "<<=" || op == ">>=") {
        return integerPromote(source);
    }
    return dest;
}

inline bool needsIntegerWiden(const Type& from, const Type& to) {
    return isIntegral(from) && isIntegral(to)
            && from.getSize() > 0 && to.getSize() > from.getSize();
}

inline bool needsNumericConvert(const Type& from, const Type& to) {
    // Bool destination is 6.3.1.2 (0/1), not float/int truncation.
    if (isBoolean(to)) {
        return false;
    }
    if (isComplex(from) || isComplex(to)) {
        return !from.equivalentTo(to);
    }
    const bool floatInt = (isFloating(from) && isIntegral(to))
            || (isIntegral(from) && isFloating(to));
    const bool floatWidth = isFloating(from) && isFloating(to)
            && from.getSize() != to.getSize();
    return floatInt || floatWidth || needsIntegerWiden(from, to);
}

// Usual arithmetic conversions: if either side is complex, convert both to
// complex of the UAC of the corresponding reals. Otherwise long double wins;
// else double; else float; else integer promotions and wider (unsigned-over-signed).
inline Type usualArithmeticResult(const Type& left, const Type& right) {
    if (isComplex(left) || isComplex(right)) {
        return complexOfReal(usualArithmeticResult(correspondingReal(left), correspondingReal(right)));
    }
    if (isFloating(left) || isFloating(right)) {
        if (isLongDouble(left) || isLongDouble(right)) {
            return longDoubleFloating();
        }
        if (isDouble(left) || isDouble(right)) {
            return doubleFloating();
        }
        return floating();
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

// C 6.3.1.2: any scalar becomes 0 or 1. Dest must be bool; source must not.
inline bool needsBoolConvert(const Type& from, const Type& to) {
    return isBoolean(to) && !isBoolean(from) && isProductScalar(from);
}

inline bool needsConversion(const Type& from, const Type& to) {
    return needsBoolConvert(from, to) || needsNumericConvert(from, to);
}

inline long convertScalarConstant(const Type& dest, long value) {
    if (isBoolean(dest)) {
        return value != 0;
    }
    return value;
}

// Operand compatibility after array/function decay (not assignment).
bool productValueCompatible(const Type& a, const Type& b);

// Git-shaped assign gate (assignment / init / call args). Not a pure subset of
// productValueCompatible: dest arrays never assign; source arrays decay;
// incomplete dest rejected; function designators only into function-pointer dest;
// null-integer into pointers.
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
    // True when base is array or pointer (stride may be 0 for empty complete records).
    bool ok { false };

    bool valid() const { return ok; }
};

// Byte size of one index step through a value of type t (0 for empty complete records).
// For array types this is the whole array size (e.g. sizeof(int[3]) for p where p is int(*)[3]).
inline int objectStrideBytes(const Type& t) {
    return t.getSize();
}

// Given the C type of the subscript base (array or pointer).
inline ArraySubscriptInfo arraySubscriptInfo(const Type& baseType) {
    ArraySubscriptInfo info;
    if (baseType.isArray()) {
        info.elementType = baseType.getElementType();
        // Index steps by sizeof(element), not sizeof(the whole array).
        info.elementStride = objectStrideBytes(info.elementType);
        info.baseIsArray = true;
        info.ok = true;
    } else if (baseType.isPointer()) {
        info.elementType = baseType.dereference();
        // p is T(*)[N]: stride is sizeof(T[N]); otherwise sizeof(pointee).
        info.elementStride = objectStrideBytes(info.elementType);
        info.baseIsArray = false;
        info.ok = true;
    } else {
        info.elementType = voidType();
        info.elementStride = 0;
        info.baseIsArray = false;
        info.ok = false;
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
        info.ok = true;
        return info;
    }
    ArraySubscriptInfo sub = arraySubscriptInfo(expressionType);
    if (!sub.valid() && valueType.isPointer()) {
        ArraySubscriptInfo info;
        info.elementType = valueType.dereference();
        info.elementStride = objectStrideBytes(info.elementType);
        info.baseIsArray = false;
        info.ok = true;
        return info;
    }
    return sub;
}

} // namespace type

#endif // TYPES_TYPEQUERY_H_
