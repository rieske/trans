#include "ArrayDeclarator.h"

#include <limits>
#include <stdexcept>

#include "AbstractSyntaxTreeVisitor.h"
#include "VlaExpressionTable.h"
#include "types/Type.h"

namespace ast {

ArrayDeclarator::ArrayDeclarator(std::unique_ptr<DirectDeclarator> declarator,
        std::unique_ptr<Expression> subscriptExpression, VlaExpressionTable* table) :
        DirectDeclarator(declarator->getName(), declarator->getContext()),
        subscriptExpression { std::move(subscriptExpression) },
        baseDeclarator { std::move(declarator) } {
    if (!this->subscriptExpression) {
        return;
    }
    long length = 0;
    if (this->subscriptExpression->foldToHostLong(length)) {
        if (length >= 0 && length <= static_cast<long>(std::numeric_limits<int>::max())) {
            setArraySize(length);
        }
        return;
    }
    if (!table) {
        throw std::logic_error { "VLA bound expression requires a VlaExpressionTable" };
    }
    vlaBound_ = std::make_shared<type::VlaBound>();
    table->bind(vlaBound_.get(), this->subscriptExpression);
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

type::Type ArrayDeclarator::getFundamentalType(std::vector<Pointer> indirection, const type::Type& baseType) const {
    type::Type elementType = baseType;
    for (Pointer pointer : indirection) {
        elementType = type::pointer(elementType, pointer.getQualifiers());
    }
    if (hasArraySize()) {
        return baseDeclarator->getFundamentalType({},
                type::array(elementType, static_cast<int>(getArraySize())));
    }
    if (vlaBound_) {
        return baseDeclarator->getFundamentalType({}, type::variableArray(elementType, vlaBound_));
    }
    if (!subscriptExpression) {
        return baseDeclarator->getFundamentalType({}, type::incompleteArray(elementType));
    }
    // Invalid ICE (negative / too large) keeps a zero-length complete shell.
    return baseDeclarator->getFundamentalType({}, type::array(elementType, 0));
}

} // namespace ast
