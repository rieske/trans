#ifndef TYPES_TYPEQUERY_H_
#define TYPES_TYPEQUERY_H_

#include "Type.h"

#include <string>

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

// Void, bare function, or incomplete record (not pointer-to-incomplete).
// Shared definition used by sizeof and member/element completeness checks.
inline bool isIncompleteObjectType(const Type& t) {
    return t.isVoid() || isBareFunction(t) || t.isIncompleteRecord();
}

// Same predicate as isIncompleteObjectType; name documents member/element sites.
inline bool isIncompleteMemberOrElementType(const Type& t) {
    return isIncompleteObjectType(t);
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

// Memory access width for loads/stores through pointers / struct fields.
// Natural size for packed 1/2/4-byte fields; otherwise a full qword.
inline int memoryAccessSizeBytes(const Type& t) {
    int size = t.getSize();
    if (size == 1 || size == 2 || size == 4) {
        return size;
    }
    return 8;
}

// Byte size for stack/global Value homes (empty types still get a word).
inline int valueSizeBytes(const Type& t) {
    int size = t.getSize();
    return size <= 0 ? 8 : size;
}

// Signedness for sub-word memory loads (sign-extend vs zero-extend).
// Only signed integral primitives sign-extend.
inline bool memoryAccessIsSigned(const Type& t) {
    if (!isIntegral(t)) {
        return false;
    }
    return t.getPrimitive().isSigned();
}

// Signedness for live Values / stack homes (SAR default, EOF -1 comparisons).
// Signed integrals report their signedness; non-integrals (incl. pointers) default signed.
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

inline bool needsNumericConvert(const Type& from, const Type& to) {
    const bool floatInt = (isFloating(from) && isIntegral(to))
            || (isIntegral(from) && isFloating(to));
    const bool floatWidth = isFloating(from) && isFloating(to)
            && from.getSize() != to.getSize();
    return floatInt || floatWidth;
}

// Usual arithmetic conversions: any double wins; else float; else integer promotions
// and wider (unsigned-over-signed) wins.
inline Type usualArithmeticResult(const Type& left, const Type& right) {
    if (isFloating(left) || isFloating(right)) {
        if ((isFloating(left) && left.getSize() >= 8)
                || (isFloating(right) && right.getSize() >= 8)) {
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

// Operand compatibility after array/function decay (not assignment).
// Comparison / logical / bitwise / conditional arms (with integral check at call site).
//
// Record rule (intentionally looser than productAssignFrom):
//   any two records are value-compatible (no structureBodyIdentity check).
// Assign requires shared body identity via productAssignFrom — do not "fix"
// this to identity or assignment will silently change comparison semantics.
bool productValueCompatible(const Type& a, const Type& b);

// Git-shaped assign gate (assignment / init / call args). Not a pure subset of
// productValueCompatible. Implementation is named allowances only:
//   - reject void / bare-function dest / incomplete dest
//   - array T[N] decays for the comparison (member arrays keep T[N] on Type)
//   - productLoosePointerToPointer (any pointer <- any pointer; permanent product rule)
//   - bare function designator -> pointer-to-function only (not void*/data pointers)
//   - null integer -> pointer
//   - transparentUnionOfPointers <-> pointer (sockaddr-style; not any record)
//   - same structureBodyIdentity for record <- record
//   - else both product scalars
// Use ONLY for assignment, initialization, and call arguments.
bool productAssignFrom(const Type& dest, const Type& source);

// Alias kept for existing call sites (same policy as productAssignFrom).
inline bool productCanAssignFrom(const Type& dest, const Type& source) {
    return productAssignFrom(dest, source);
}

// Scalar arithmetic (* / % and non-pointer +/-): both arithmetic types.
bool productArithmeticCompatible(const Type& a, const Type& b);

// Diagnostic text for a failed product assign (call only when canAssign is false).
std::string productAssignFailureMessage(const Type& dest, const Type& source);

// Array subscript element info for SA (shared policy, not mid-visitor specials).
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

// C 6.4.5 / 6.5.3.4: sizeof "abc" is the array size including NUL, not sizeof(char*).
// The AST types string literals as const char* for decay; sizeof must use the
// lexical length. `token` is the scanner string token (quotes included).
// Implemented via util::stringLiteralArrayLength — declared here for SA policy.
int sizeofStringLiteralTokenBytes(const std::string& token);

} // namespace type

#endif // TYPES_TYPEQUERY_H_
