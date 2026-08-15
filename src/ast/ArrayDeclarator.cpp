#include "ArrayDeclarator.h"

#include <limits>
#include <stdexcept>

#include "AbstractSyntaxTreeVisitor.h"
#include "types/Type.h"

namespace ast {

ArrayDeclarator::ArrayDeclarator(std::unique_ptr<DirectDeclarator> declarator,
        std::unique_ptr<Expression> subscriptExpression) :
        DirectDeclarator(declarator->getName(), declarator->getContext()),
        subscriptExpression { std::move(subscriptExpression) },
        baseDeclarator { std::move(declarator) } {
}

void ArrayDeclarator::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void ArrayDeclarator::visitBaseDeclarator(AbstractSyntaxTreeVisitor& visitor) {
    baseDeclarator->accept(visitor);
}

void ArrayDeclarator::forEachArrayDeclarator(const std::function<void(ArrayDeclarator&)>& fn) {
    baseDeclarator->forEachArrayDeclarator(fn);
    fn(*this);
}

const FunctionDeclarator* ArrayDeclarator::innermostFunctionDeclarator() const {
    return baseDeclarator->innermostFunctionDeclarator();
}

ArrayBoundFold ArrayDeclarator::foldOwnBound() {
    if (!subscriptExpression || hasArraySize()) {
        return ArrayBoundFold::Complete;
    }
    long length = 0;
    if (!subscriptExpression->evaluateConstant(length)) {
        return ArrayBoundFold::Unfixed;
    }
    if (length < 0) {
        return ArrayBoundFold::Negative;
    }
    if (length > static_cast<long>(std::numeric_limits<int>::max())) {
        setArraySize(0);
        return ArrayBoundFold::TooLarge;
    }
    setArraySize(length);
    return ArrayBoundFold::Complete;
}

void ArrayDeclarator::setArraySize(long size) {
    arraySize = size;
    arraySizeSet = true;
}

bool ArrayDeclarator::hasArraySize() const {
    return arraySizeSet;
}

long ArrayDeclarator::getArraySize() const {
    return arraySize;
}

type::Type ArrayDeclarator::getFundamentalType(std::vector<Pointer> indirection, const type::Type& baseType) const {
    type::Type elementType = baseType;
    for (Pointer pointer : indirection) {
        elementType = type::pointer(elementType, pointer.getQualifiers());
    }
    // Prefer size folded in semantic analysis. Unsized `T a[]` is incomplete.
    // Invalid bounds keep a zero-length complete shell after a semantic error.
    if (!hasArraySize() && !subscriptExpression) {
        return baseDeclarator->getFundamentalType({}, type::incompleteArray(elementType));
    }
    long length = 0;
    if (hasArraySize()) {
        length = getArraySize();
    } else if (subscriptExpression && subscriptExpression->evaluateConstant(length) && length >= 0) {
        // Fallback when getFundamentalType is used without a prior semantic visit.
    } else if (subscriptExpression) {
        return baseDeclarator->getFundamentalType({}, type::variableArray(elementType, subscriptExpression));
    }
    if (length > static_cast<long>(std::numeric_limits<int>::max())) {
        length = 0;
    }
    // type::array may throw std::invalid_argument (overflow / incomplete element);
    // semantic analysis catches that when building declaration types.
    return baseDeclarator->getFundamentalType({}, type::array(elementType, static_cast<int>(length)));
}

} // namespace ast
