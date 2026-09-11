#ifndef TYPES_TYPEQUERY_H_
#define TYPES_TYPEQUERY_H_

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "Operator.h"
#include "Type.h"
#include "TypeConstraint.h"

namespace type {

// Pointer/function combinations (isPointerToFunction, …). A function type is
// Type::isFunction(); a pointer is its own kind and does not bleed Function.

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

// Classify `left op right` for Add/Sub. Result type is the pointer type or int (ptrdiff).
PointerArithmeticInfo classifyPointerArithmetic(const Type& left, const Type& right,
        ArithmeticOp op);

inline bool isPointerToFunction(const Type& t) {
    return t.isPointer() && t.dereference().isFunction();
}

// Void, bare function, incomplete record, or incomplete array (not pointer-to-incomplete).
// Shared definition used by sizeof and member/element completeness checks.
inline bool isIncompleteObjectType(const Type& t) {
    return incompleteArrayElement(t);
}

// VLA, or array whose element has a runtime size. Pointer-to-VLA is not included:
// sizeof(int (*)[n]) is an ICE (pointer width).
inline bool hasRuntimeSize(const Type& t) {
    if (t.isVariableArray()) {
        return true;
    }
    if (t.isArray()) {
        return hasRuntimeSize(t.getElementType());
    }
    return false;
}

// Parse laid out members but left the record incomplete: a bound was not an ICE
// yet, or a nested record is itself still tentative.
inline bool isTentativeRecord(const Type& t) {
    return t.isRecord() && !t.isCompleteRecord() && t.memberCount() > 0;
}

// VLA whose size cannot be formed: a [*] layer, or an array of such.
inline bool hasUnspecifiedVlaSize(const Type& t) {
    if (t.isVariableArray()) {
        auto bound = t.vlaBound();
        return !bound || bound->unspecified || hasUnspecifiedVlaSize(t.getElementType());
    }
    if (t.isArray()) {
        return hasUnspecifiedVlaSize(t.getElementType());
    }
    return false;
}

inline bool hasComputableRuntimeSize(const Type& t) {
    return hasRuntimeSize(t) && !hasUnspecifiedVlaSize(t);
}

// Sizeof of an object type when it is an ICE. GNU sizeof(function) and sizeof(void)
// are 1; ISO treats both as incomplete. VM types are complete but not an ICE
// (nullopt, not an error).
std::optional<int> sizeofObject(const Type& t, bool gnu);

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

// C 6.7.6.3 parameter type adjustment only (does not drop top-level cv).
// Array of T -> pointer to T; function returning T -> pointer to function returning T.
inline Type adjustedParameterType(Type t) {
    if (t.isArray()) {
        return pointer(t.getElementType());
    }
    if (t.isFunction()) {
        return pointer(t);
    }
    return t;
}

// Record type for `.` / `->` (arrow base is lvalue-converted first).
std::optional<Type> memberAccessRecordType(const Type& baseType, bool arrow);

// Member type of `base.member` / `base->member`, or nullopt if ill-formed / unknown member.
std::optional<Type> memberAccessResult(const Type& baseType, bool arrow,
        const std::string& memberName);

// Integer promotions (C 6.3.1.1): types narrower than int convert to int.
Type integerPromote(const Type& t);

// C 6.5.2.2: integer promotions, then float -> double. Other types unchanged.
Type defaultArgPromote(const Type& t);

// Assignment RHS convert dest. <<= >>=: integer-promote the count, not the LHS type.
// Pointer +=/-=: the integer stays an integer (C 6.5.16.2); do not convert it to the pointer type.
Type assignmentConvertTarget(AssignOp op, const Type& dest, const Type& source);

inline bool needsIntegerWiden(const Type& from, const Type& to) {
    return isIntegral(from) && isIntegral(to)
            && from.getSize() > 0 && to.getSize() > from.getSize();
}

inline bool needsIntegerNarrow(const Type& from, const Type& to) {
    return isIntegral(from) && isIntegral(to) && !isBoolean(to)
            && to.getSize() > 0 && from.getSize() > to.getSize();
}

// Integer-to-pointer (6.3.2.3): widen a narrower integer to pointer width.
inline bool needsIntegerToPointerExtend(const Type& from, const Type& to) {
    return isIntegral(from) && to.isPointer()
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
    return floatInt || floatWidth || needsIntegerWiden(from, to) || needsIntegerNarrow(from, to);
}

// Usual arithmetic conversions: if either side is complex, convert both to
// complex of the UAC of the corresponding reals. Otherwise long double wins;
// else double; else float; else integer promotions and wider (unsigned-over-signed).
Type usualArithmeticResult(const Type& left, const Type& right);

// Result type of `left op right` after lvalue conversion of both operands.
// Pointer forms use classifyPointerArithmetic; pure arithmetic uses UAC.
// nullopt: invalid pointer arithmetic or non-arithmetic operands.
std::optional<Type> arithmeticExpressionResult(const Type& leftRaw, const Type& rightRaw,
        ArithmeticOp op);

// Result type of `cond ? a : b` after lvalue conversion of both arms (C 6.5.15).
// Product-loose pointers: any two pointers are compatible; prefer void* when either
// pointee is void. Integral 0 with a pointer yields the pointer type.
// nullopt: arms have no product-compatible common type.
std::optional<Type> conditionalResultType(const Type& trueRaw, const Type& falseRaw);

// Primitive or pointer (caller must decay arrays/functions if desired).
inline bool isProductScalar(const Type& t) {
    return t.kind() == TypeKind::Primitive || t.isPointer();
}

// C 6.3.1.2: any scalar becomes 0 or 1. Dest must be bool; source must not.
inline bool needsBoolConvert(const Type& from, const Type& to) {
    return isBoolean(to) && !isBoolean(from) && isProductScalar(from);
}

inline bool needsConversion(const Type& from, const Type& to) {
    return needsBoolConvert(from, to) || needsNumericConvert(from, to)
            || needsIntegerToPointerExtend(from, to);
}

// One _Generic association after its type-name is resolved (or failed).
// isDefault: default association. type: typed arm; null if unresolved.
struct GenericArmView {
    bool isDefault { false };
    const Type* type { nullptr };
};

enum class GenericSelectionStatus {
    Ok,
    NoMatch,
    MultipleMatches,
};

struct GenericSelectionChoice {
    GenericSelectionStatus status { GenericSelectionStatus::NoMatch };
    std::optional<std::size_t> index;
};

// First matching typed arm, else first default. Unresolved typed arms do not match.
GenericSelectionChoice selectGenericAssociation(
        const Type& convertedControlling, const std::vector<GenericArmView>& arms);

// Git-shaped assign gate on types alone (assignment / init / call args).
// Dest arrays never assign; source arrays decay; incomplete dest rejected;
// function designators only into function-pointer dest; integral 0 into pointers.
// Expression-sensitive null forms ((void*)0) live in SA productAssignOk / checkAssign.
bool productAssignFrom(const Type& dest, const Type& source);

// Alias kept for existing call sites (same policy as productAssignFrom).
inline bool productCanAssignFrom(const Type& dest, const Type& source) {
    return productAssignFrom(dest, source);
}

// Scalar arithmetic (* / % and non-pointer +/-): both arithmetic types.
inline bool productArithmeticCompatible(const Type& a, const Type& b) {
    return isArithmeticType(a) && isArithmeticType(b);
}

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

inline bool isSubscriptBase(const Type& expressionType, const Type& valueType) {
    return expressionType.isArray() || expressionType.isPointer()
            || valueType.isArray() || valueType.isPointer();
}

// Byte size of one index step through a value of type t (0 for empty complete records).
// For array types this is the whole array size (e.g. sizeof(int[3]) for p where p is int(*)[3]).
inline int objectStrideBytes(const Type& t) {
    return t.getSize();
}

// Given the C type of the subscript base (array or pointer).
ArraySubscriptInfo arraySubscriptInfo(const Type& baseType);

// Dual-type subscript: expression type may still be T[N] while value type is
// already a decayed pointer.
ArraySubscriptInfo arraySubscriptInfo(const Type& expressionType, const Type& valueType);

} // namespace type

#endif // TYPES_TYPEQUERY_H_
