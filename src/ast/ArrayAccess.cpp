#include "ArrayAccess.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "Operator.h"

namespace ast {

ArrayAccess::ArrayAccess(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<Expression> subscriptExpression) :
        DoubleOperandExpression(std::move(postfixExpression), std::move(subscriptExpression), std::unique_ptr<Operator> { new Operator("[]") }) {
    // Element access is always an lvalue when the base is addressable (array or pointer).
    lval = true;
}

void ArrayAccess::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void ArrayAccess::setLvalue(semantic_analyzer::ValueEntry lvalue) {
    this->lvalue = std::make_unique<semantic_analyzer::ValueEntry>(lvalue);
}

semantic_analyzer::ValueEntry* ArrayAccess::getLvalue() const {
    return lvalue.get();
}

semantic_analyzer::ValueEntry* ArrayAccess::getLvalueSymbol() const {
    return getLvalue();
}

void ArrayAccess::setElementSize(int sizeInBytes) {
    elementSize = sizeInBytes;
}

int ArrayAccess::getElementSize() const {
    return elementSize;
}

void ArrayAccess::setBaseIsArray(bool value) {
    baseIsArrayFlag = value;
}

bool ArrayAccess::baseIsArray() const {
    return baseIsArrayFlag;
}

void ArrayAccess::setYieldsAddress(bool value) {
    yieldsAddressFlag = value;
}

bool ArrayAccess::yieldsAddress() const {
    return yieldsAddressFlag;
}

} // namespace ast
