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

// Product-loose null constant: integral (including null pointer constant 0).
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
        std::string name;
        Type memberTy { voidType() };
        int off = 0;
        if (!t.memberAt(i, name, memberTy, off)) {
            return false;
        }
        if (!memberTy.isPointer()) {
            return false;
        }
    }
    return true;
}

// Named product allowances (see TypeQuery.h productAssignFrom contract).

bool allowNullIntegerToPointer(const Type& dest, const Type& source) {
    return dest.isPointer() && isNullConstantCandidate(source);
}

// Permanent product rule (not ISO): any pointer dest may take any pointer source.
// Needed for git-shaped C after host preprocess (void*/typed pointer mixing without
// full C conversion ranks). Contract: Type.productLoosePointerAssignIsPermanent and
// ProductContracts.loosePointerAssignCompiles.
bool allowProductLoosePointerToPointer(const Type& dest, const Type& source) {
    return dest.isPointer() && source.isPointer();
}

// Bare function designator only into pointer-to-function (not void* / data pointers).
bool allowFunctionDesignatorToPointer(const Type& dest, const Type& sourceBeforeDecay) {
    return isPointerToFunction(dest) && isBareFunction(sourceBeforeDecay);
}

bool allowSameRecordBody(const Type& dest, const Type& source) {
    if (!dest.isRecord() || !source.isRecord()) {
        return false;
    }
    const void* id = dest.structureBodyIdentity();
    return id != nullptr && id == source.structureBodyIdentity();
}

// Pointer <-> complete union-of-pointer-arms (e.g. __CONST_SOCKADDR_ARG).
bool allowTransparentUnionPointer(const Type& dest, const Type& source) {
    if (dest.isPointer() && isTransparentUnionOfPointers(source)) {
        return true;
    }
    if (source.isPointer() && isTransparentUnionOfPointers(dest)) {
        return true;
    }
    return false;
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

    // Array decay for assign comparison only (member arrays keep T[N] on the Type).
    Type d = dest.isArray() ? dest.decayArray() : dest;
    Type s = source.isArray() ? source.decayArray() : source;

    if (allowProductLoosePointerToPointer(d, s)
            || allowFunctionDesignatorToPointer(d, source)
            || allowNullIntegerToPointer(d, s)
            || allowTransparentUnionPointer(d, s)
            || allowSameRecordBody(d, s)) {
        return true;
    }

    // Function designator / pointer-to-function used as a non-pointer value.
    if (isBareFunction(source) || isPointerToFunction(s)) {
        return false;
    }

    return isProductScalar(d) && isProductScalar(s);
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
