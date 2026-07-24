#include "ArrayDeclarator.h"

#include <limits>
#include <stdexcept>

#include "AbstractSyntaxTreeVisitor.h"
#include "Expression.h"
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

type::Type ArrayDeclarator::getFundamentalType(std::vector<Pointer> indirection, const type::Type& baseType) {
    type::Type elementType = baseType;
    for (Pointer pointer : indirection) {
        elementType = type::pointer(elementType, pointer.getQualifiers());
    }
    // Prefer size folded in semantic analysis. Invalid bounds yield a zero-length
    // array type after a semantic error was already reported (no exception).
    long length = 0;
    if (arraySizeSet) {
        length = arraySize;
    } else if (subscriptExpression && subscriptExpression->evaluateConstant(length) && length >= 0) {
        // Fallback when getFundamentalType is used without a prior semantic visit.
    } else {
        length = 0;
    }
    if (length > static_cast<long>(std::numeric_limits<int>::max())) {
        length = 0;
    }
    return baseDeclarator->getFundamentalType({}, type::array(elementType, static_cast<int>(length)));
}

} // namespace ast
