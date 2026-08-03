#include "ArrayDeclarator.h"

#include <limits>
#include <stdexcept>

#include "AbstractSyntaxTreeVisitor.h"
#include "types/Type.h"
#include "util/ProductApprox.h"

namespace ast {

ArrayDeclarator::ArrayDeclarator(std::unique_ptr<DirectDeclarator> declarator, std::unique_ptr<Expression> subscriptExpression) :
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

DirectDeclarator& ArrayDeclarator::getBaseDeclarator() const {
    return *baseDeclarator;
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
    if (!subscriptExpression->foldToHostLong(length)) {
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

void ArrayDeclarator::foldArrayBoundSizeofs(const std::function<void(Expression*)>& foldSizeof) {
    if (subscriptExpression) {
        foldSizeof(subscriptExpression.get());
        long size = 0;
        if (subscriptExpression->foldToHostLong(size) && size > 0) {
            setArraySize(size);
        }
    }
    baseDeclarator->foldArrayBoundSizeofs(foldSizeof);
}

type::Type ArrayDeclarator::getFundamentalType(std::vector<Pointer> indirection, const type::Type& baseType) const {
    // C: `int *a[N]` is array of N pointers (indirection applies to the element type).
    // Nested brackets compose from the inside out: T a[4][20] parses as
    // ArrayDeclarator(20, ArrayDeclarator(4, id)) and must become array[4] of array[20] of T
    // (outer dimension applied by the nested declarator).
    type::Type elementType = baseType;
    for (Pointer pointer : indirection) {
        elementType = type::pointer(elementType, pointer.getQualifiers());
    }
    // Unsized T name[] (and flexible members) stay incomplete until init completes them.
    if (!subscriptExpression) {
        return baseDeclarator->getFundamentalType({}, type::incompleteArray(elementType));
    }
    // Prefer size folded during semantic analysis (sizeof(arr[0]) needs a visit first).
    // Unfixed bounds must not be stored as size 0: that would look complete and hide
    // parameter VLAs. Decay those to pointer-to-element (C parameter adjustment).
    long length = 0;
    if (hasArraySize()) {
        length = getArraySize();
    } else if (subscriptExpression && subscriptExpression->foldToHostLong(length)) {
        if (length < 0) {
            length = product_approx::clampNegativeArrayBoundForBuildAssert();
        }
    } else if (subscriptExpression) {
        return baseDeclarator->getFundamentalType({}, type::variableArray(elementType, subscriptExpression));
    }
    if (length > static_cast<long>(std::numeric_limits<int>::max())) {
        throw std::invalid_argument { "array size is too large" };
    }
    // GCC zero-length arrays appear in system headers as flexible array members.
    // type::array also rejects stride*count overflow (INT_MAX-sized int arrays, etc.).
    return baseDeclarator->getFundamentalType({}, type::array(elementType, static_cast<int>(length)));
}

} // namespace ast
