#include "TypeQuery.h"
#include "util/StringLiteralDecode.h"

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

// Operand-side record pairs: any two records (no body identity). Documented in
// TypeQuery.h — intentionally looser than productAssignFrom record-record.
bool recordsCompatibleForValue(const Type& a, const Type& b) {
    return a.isRecord() && b.isRecord();
}

// Type-only null constant: integral 0. ((void*)0) is handled in SA with a source expression.
bool isNullConstantCandidate(const Type& t) {
    return isIntegral(t);
}

// Complete union whose every stored arm is a pointer (glibc transparent-union shape).
bool isTransparentUnionOfPointers(const Type& t) {
    if (!t.isUnion() || !t.isCompleteRecord()) {
        return false;
    }
    const int n = t.memberCount();
    if (n < 1) {
        return false;
    }
    for (int i = 0; i < n; ++i) {
        auto member = memberAt(t, i);
        if (!member || !member->type.isPointer()) {
            return false;
        }
    }
    return true;
}

// Named product allowances (see TypeQuery.h productAssignFrom contract).

bool allowNullIntegerToPointer(const Type& dest, const Type& source) {
    return dest.isPointer() && isNullConstantCandidate(source);
}

// Permanent product rule (not ISO): data pointer dest may take any pointer source.
// Function-pointer dest is tighter (master #176): designator, pointer-to-function,
// or null constant only — not void*/arbitrary data pointers as type-only null.
// Contract: Type.productLoosePointerAssignIsPermanent and
// ProductContracts.loosePointerAssignCompiles.
bool allowProductLoosePointerToPointer(const Type& dest, const Type& source) {
    return dest.isPointer() && !isPointerToFunction(dest) && source.isPointer();
}

// Bare function designator or pointer-to-function into pointer-to-function.
bool allowFunctionPointerAssign(const Type& dest, const Type& sourceBeforeDecay, const Type& source) {
    if (!isPointerToFunction(dest)) {
        return false;
    }
    return isBareFunction(sourceBeforeDecay) || isPointerToFunction(source);
}

bool allowSameRecordBody(const Type& dest, const Type& source) {
    if (!dest.isRecord() || !source.isRecord()) {
        return false;
    }
    const void* id = dest.structureBodyIdentity();
    return id != nullptr && id == source.structureBodyIdentity();
}

// Union-of-pointer-arms used as a pointer value (glibc sockaddr helpers).
// Assignment *into* a union dest is only dest.isTransparentUnion() (GNU attribute).
bool allowTransparentUnionPointer(const Type& dest, const Type& source) {
    return dest.isPointer() && isTransparentUnionOfPointers(source);
}

} // namespace

bool productValueCompatible(const Type& a, const Type& b) {
    Type la = productDecay(a);
    Type ra = productDecay(b);
    if (la.isVoid() || ra.isVoid()) {
        return false;
    }
    // Operand compatibility: any two records (no structureBodyIdentity).
    // Assignment uses allowSameRecordBody instead — see TypeQuery.h.
    if (la.isRecord() || ra.isRecord()) {
        return recordsCompatibleForValue(la, ra);
    }
    return isProductScalar(la) && isProductScalar(ra);
}

bool productAssignFrom(const Type& dest, const Type& source) {
    // Structured product assign: reject hard cases, then named allowances only.
    if (dest.isVoid() || isBareFunction(dest)) {
        return false;
    }
    if (dest.isIncompleteRecord()) {
        return false;
    }

    if (dest.isArray()) {
        return false;
    }
    // Source array decay for assign comparison only (member arrays keep T[N] on the Type).
    Type s = source.isArray() ? source.decayArray() : source;

    if (dest.isTransparentUnion()) {
        if (isNullConstantCandidate(s)) {
            return true;
        }
        for (const auto& member : dest.getMembers()) {
            if (member.type && productAssignFrom(*member.type, s)) {
                return true;
            }
        }
        return false;
    }

    // Function-pointer dest: designator, pointer-to-function, or integral null only.
    // void* / data pointers are not type-only null; SA productAssignOk folds ((void*)0).
    if (isPointerToFunction(dest)) {
        return allowFunctionPointerAssign(dest, source, s)
                || allowNullIntegerToPointer(dest, s);
    }

    if (allowProductLoosePointerToPointer(dest, s)
            || allowNullIntegerToPointer(dest, s)
            || allowTransparentUnionPointer(dest, s)
            || allowSameRecordBody(dest, s)) {
        return true;
    }

    // Function designator / pointer-to-function used as a non-pointer value.
    if (isBareFunction(source) || isPointerToFunction(s)) {
        return false;
    }

    return isProductScalar(dest) && isProductScalar(s);
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

int sizeofStringLiteralTokenBytes(const std::string& token) {
    return util::stringLiteralArrayLength(token);
}

} // namespace type
