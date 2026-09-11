#include "TypeQuery.h"

namespace type {

namespace {

Type productDecay(Type t) {
    if (t.isArray()) {
        return t.decayArray();
    }
    if (t.isFunction()) {
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
    if (dest.isVoid()) {
        return source.isVoid();
    }
    if (dest.isFunction()) {
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
        if (src.isFunction() || isPointerToFunction(src)) {
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
    if (src.isFunction() || isPointerToFunction(src)) {
        return false;
    }
    if (dest.isRecord() || src.isRecord()) {
        return recordsCompatible(dest, src);
    }
    return isProductScalar(dest) && isProductScalar(src);
}

std::string productAssignFailureMessage(const Type& dest, const Type& source) {
    if ((source.isFunction() || isPointerToFunction(source)) && !isPointerToFunction(dest)) {
        return "function designator used as a value is not supported";
    }
    return "type mismatch: can't convert " + source.to_string() + " to " + dest.to_string();
}

Type integerPromote(const Type& t) {
    if (!isIntegral(t)) {
        return t;
    }
    if (t.getSize() > 0 && t.getSize() < 4) {
        return signedInteger();
    }
    return t;
}

Type defaultArgPromote(const Type& t) {
    if (isFloat(t)) {
        return doubleFloating();
    }
    return integerPromote(t);
}

Type assignmentConvertTarget(AssignOp op, const Type& dest, const Type& source) {
    if (op == AssignOp::ShlAssign || op == AssignOp::ShrAssign) {
        return integerPromote(source);
    }
    if ((op == AssignOp::AddAssign || op == AssignOp::SubAssign)
            && dest.isPointer() && isIntegralScalar(source)) {
        return integerPromote(source);
    }
    return dest;
}

// Classify `left op right` for Add/Sub. Result type is the pointer type or int (ptrdiff).
PointerArithmeticInfo classifyPointerArithmetic(const Type& left, const Type& right,
        ArithmeticOp op) {
    PointerArithmeticInfo info;
    if (op != ArithmeticOp::Add && op != ArithmeticOp::Sub) {
        return info;
    }
    if (!left.isPointer() && !right.isPointer()) {
        return info;
    }
    if (op == ArithmeticOp::Add && left.isPointer() && isIntegralScalar(right)) {
        info.form = PointerArithmeticForm::PtrPlusInt;
        info.resultType = left;
        info.strideBytes = pointerElementStride(left);
        return info;
    }
    if (op == ArithmeticOp::Add && isIntegralScalar(left) && right.isPointer()) {
        info.form = PointerArithmeticForm::IntPlusPtr;
        info.resultType = right;
        info.strideBytes = pointerElementStride(right);
        return info;
    }
    if (op == ArithmeticOp::Sub && left.isPointer() && isIntegralScalar(right)) {
        info.form = PointerArithmeticForm::PtrMinusInt;
        info.resultType = left;
        info.strideBytes = pointerElementStride(left);
        return info;
    }
    if (op == ArithmeticOp::Sub && left.isPointer() && right.isPointer()) {
        info.form = PointerArithmeticForm::PtrMinusPtr;
        info.resultType = signedInteger();
        info.strideBytes = pointerElementStride(left);
        return info;
    }
    info.form = PointerArithmeticForm::Invalid;
    return info;
}

std::optional<int> sizeofObject(const Type& t, bool gnu) {
    if (t.isVoid()) {
        return gnu ? std::optional<int> { 1 } : std::nullopt;
    }
    if (t.isFunction()) {
        if (gnu) {
            return 1;
        }
        return std::nullopt;
    }
    if (isIncompleteObjectType(t) || hasRuntimeSize(t)) {
        return std::nullopt;
    }
    return t.getSize();
}

Type usualArithmeticResult(const Type& left, const Type& right) {
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

std::optional<Type> arithmeticExpressionResult(const Type& leftRaw, const Type& rightRaw,
        ArithmeticOp op) {
    const Type left = afterLvalueConversion(leftRaw);
    const Type right = afterLvalueConversion(rightRaw);
    const PointerArithmeticInfo ptrArith = classifyPointerArithmetic(left, right, op);
    if (ptrArith.form != PointerArithmeticForm::None) {
        if (ptrArith.form == PointerArithmeticForm::Invalid) {
            return std::nullopt;
        }
        return ptrArith.resultType;
    }
    if (isArithmeticType(left) && isArithmeticType(right)) {
        return usualArithmeticResult(left, right);
    }
    return std::nullopt;
}

std::optional<Type> memberAccessRecordType(const Type& baseType, bool arrow) {
    if (arrow) {
        const Type converted = afterLvalueConversion(baseType);
        if (!converted.isPointer()) {
            return std::nullopt;
        }
        const Type pointee = converted.dereference();
        if (!pointee.isRecord()) {
            return std::nullopt;
        }
        return pointee;
    }
    if (!baseType.isRecord()) {
        return std::nullopt;
    }
    return baseType;
}

std::optional<Type> memberAccessResult(const Type& baseType, bool arrow,
        const std::string& memberName) {
    const auto record = memberAccessRecordType(baseType, arrow);
    if (!record) {
        return std::nullopt;
    }
    const auto found = lookupMember(*record, memberName);
    if (!found) {
        return std::nullopt;
    }
    return found->type;
}

std::optional<Type> conditionalResultType(const Type& trueRaw, const Type& falseRaw) {
    const Type left = afterLvalueConversion(trueRaw);
    const Type right = afterLvalueConversion(falseRaw);
    if (left.isVoid() && right.isVoid()) {
        return left;
    }
    if (isArithmeticType(left) && isArithmeticType(right)) {
        return usualArithmeticResult(left, right);
    }
    if (left.isPointer() && right.isPointer()) {
        // Function pointers do not mix with object void* as void*; keep the fnptr arm
        // (gcc extension: `cond ? free : NULL` has function-pointer type).
        if (isPointerToFunction(left)
                && (isPointerToFunction(right) || right.dereference().isVoid())) {
            return left;
        }
        if (isPointerToFunction(right)
                && (isPointerToFunction(left) || left.dereference().isVoid())) {
            return right;
        }
        if (left.dereference().isVoid()) {
            return left;
        }
        if (right.dereference().isVoid()) {
            return right;
        }
        return left;
    }
    if (left.isPointer() && isIntegral(right)) {
        return left;
    }
    if (right.isPointer() && isIntegral(left)) {
        return right;
    }
    if (left.isRecord() && right.isRecord()) {
        return left;
    }
    return std::nullopt;
}

GenericSelectionChoice selectGenericAssociation(
        const Type& convertedControlling, const std::vector<GenericArmView>& arms) {
    std::optional<std::size_t> defaultIndex;
    std::optional<std::size_t> match;
    for (std::size_t i = 0; i < arms.size(); ++i) {
        const GenericArmView& arm = arms[i];
        if (arm.isDefault) {
            if (!defaultIndex) {
                defaultIndex = i;
            }
            continue;
        }
        if (arm.type && arm.type->sameQualifiedType(convertedControlling)) {
            if (match) {
                return GenericSelectionChoice { GenericSelectionStatus::MultipleMatches, {} };
            }
            match = i;
        }
    }
    if (match) {
        return GenericSelectionChoice { GenericSelectionStatus::Ok, match };
    }
    if (defaultIndex) {
        return GenericSelectionChoice { GenericSelectionStatus::Ok, defaultIndex };
    }
    return GenericSelectionChoice { GenericSelectionStatus::NoMatch, {} };
}

ArraySubscriptInfo arraySubscriptInfo(const Type& baseType) {
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

ArraySubscriptInfo arraySubscriptInfo(const Type& expressionType, const Type& valueType) {
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
