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

type::Type ArrayDeclarator::getFundamentalType(std::vector<Pointer> indirection, const type::Type& baseType) {
    type::Type elementType = baseType;
    for (Pointer pointer : indirection) {
        elementType = type::pointer(elementType, pointer.getQualifiers());
    }
    long length = 0;
    if (!subscriptExpression || !subscriptExpression->evaluateConstant(length) || length < 0) {
        throw std::runtime_error { "array size is not a non-negative constant expression" };
    }
    if (length > static_cast<long>(std::numeric_limits<int>::max())) {
        throw std::invalid_argument { "array size is too large" };
    }
    return baseDeclarator->getFundamentalType({}, type::array(elementType, static_cast<int>(length)));
}

} // namespace ast
