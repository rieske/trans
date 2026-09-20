#include "Type.h"

namespace type {

namespace {

enum class ArrayBound {
    Exact,
    Compatible
};

bool sameShape(const Type& a, const Type& b, bool matchQualifiers, ArrayBound bounds);

bool sameShapeAfterCv(const Type& left, const Type& right, bool matchQualifiers, ArrayBound bounds) {
    if (left.kind() != right.kind()) {
        return false;
    }
    switch (left.kind()) {
    case TypeKind::Void:
        return true;
    case TypeKind::Primitive:
        return left.getPrimitive().equivalentTo(right.getPrimitive());
    case TypeKind::Pointer:
        return sameShape(left.dereference(), right.dereference(), matchQualifiers, bounds);
    case TypeKind::Array:
        if (!sameShape(left.getElementType(), right.getElementType(), matchQualifiers, bounds)) {
            return false;
        }
        if (bounds == ArrayBound::Exact) {
            return left.isIncompleteArray() == right.isIncompleteArray()
                    && left.isVariableArray() == right.isVariableArray()
                    && (left.isVariableArray() || left.getArraySize() == right.getArraySize());
        }
        if (left.isVariableArray() || right.isVariableArray()) {
            return true;
        }
        if (!left.isIncompleteArray() && !right.isIncompleteArray()) {
            return left.getArraySize() == right.getArraySize();
        }
        return true;
    case TypeKind::Function: {
        const Function& fa = left.getFunction();
        const Function& fb = right.getFunction();
        if (fa.isVariadic() != fb.isVariadic()) {
            return false;
        }
        if (!sameShape(fa.getReturnType(), fb.getReturnType(), matchQualifiers, bounds)) {
            return false;
        }
        const std::vector<Type>& aa = fa.getArguments();
        const std::vector<Type>& ba = fb.getArguments();
        if (aa.size() != ba.size()) {
            return false;
        }
        for (std::size_t i = 0; i < aa.size(); ++i) {
            if (bounds == ArrayBound::Compatible) {
                if (!sameShape(aa[i].withoutTopLevelQualifiers(), ba[i].withoutTopLevelQualifiers(),
                            matchQualifiers, bounds)) {
                    return false;
                }
            } else if (!sameShape(aa[i], ba[i], matchQualifiers, bounds)) {
                return false;
            }
        }
        return true;
    }
    case TypeKind::Struct:
    case TypeKind::Union:
        return left.structureBodyIdentity() == right.structureBodyIdentity();
    }
    return false;
}

bool sameShape(const Type& a, const Type& b, bool matchQualifiers, ArrayBound bounds) {
    if (matchQualifiers) {
        if (a.isConst() != b.isConst() || a.isVolatile() != b.isVolatile()) {
            return false;
        }
        return sameShapeAfterCv(a, b, matchQualifiers, bounds);
    }
    return sameShapeAfterCv(a.withoutTopLevelQualifiers(), b.withoutTopLevelQualifiers(),
            matchQualifiers, bounds);
}

std::vector<Qualifier> topQualifiers(const Type& t) {
    std::vector<Qualifier> quals;
    if (t.isConst()) {
        quals.push_back(Qualifier::CONST);
    }
    if (t.isVolatile()) {
        quals.push_back(Qualifier::VOLATILE);
    }
    return quals;
}

Type makeComposite(const Type& a, const Type& b) {
    switch (a.kind()) {
    case TypeKind::Array: {
        const Type element = makeComposite(a.getElementType(), b.getElementType());
        Type result = incompleteArray(element);
        if (a.isVariableArray() && b.isVariableArray()) {
            auto bound = a.vlaBound();
            auto other = b.vlaBound();
            if ((!bound || bound->unspecified) && other && !other->unspecified) {
                bound = std::move(other);
            }
            result = variableArray(element, std::move(bound));
        } else if (a.isVariableArray()) {
            result = b.isIncompleteArray() ? variableArray(element)
                    : array(element, b.getArraySize());
        } else if (b.isVariableArray()) {
            result = a.isIncompleteArray() ? variableArray(element)
                    : array(element, a.getArraySize());
        } else if (!a.isIncompleteArray()) {
            result = array(element, a.getArraySize());
        } else if (!b.isIncompleteArray()) {
            result = array(element, b.getArraySize());
        }
        return result.withQualifiers(topQualifiers(a));
    }
    case TypeKind::Pointer:
        return pointer(makeComposite(a.dereference(), b.dereference()), topQualifiers(a));
    case TypeKind::Function: {
        const Function& fa = a.getFunction();
        const Function& fb = b.getFunction();
        std::vector<Type> args;
        const std::vector<Type>& aa = fa.getArguments();
        const std::vector<Type>& ba = fb.getArguments();
        args.reserve(aa.size());
        for (std::size_t i = 0; i < aa.size(); ++i) {
            args.push_back(makeComposite(aa[i], ba[i]));
        }
        return function(makeComposite(fa.getReturnType(), fb.getReturnType()), args, fa.isVariadic())
                .withQualifiers(topQualifiers(a));
    }
    default:
        return a;
    }
}

} // namespace

bool Type::equivalentTo(const Type& other) const {
    return sameShape(*this, other, false, ArrayBound::Exact);
}

bool Type::sameQualifiedType(const Type& other) const {
    return sameShape(*this, other, true, ArrayBound::Exact);
}

bool Type::compatibleWith(const Type& other) const {
    return sameShape(*this, other, true, ArrayBound::Compatible);
}

std::optional<Type> Type::composite(const Type& other) const {
    if (!compatibleWith(other)) {
        return std::nullopt;
    }
    if (sameQualifiedType(other)) {
        return *this;
    }
    return makeComposite(*this, other);
}

bool Type::sameUnqualifiedType(const Type& other) const {
    return withoutTopLevelQualifiers().sameQualifiedType(other.withoutTopLevelQualifiers());
}

} // namespace type
