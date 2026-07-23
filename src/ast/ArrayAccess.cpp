#include "ArrayAccess.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "Operator.h"

namespace ast {

ArrayAccess::ArrayAccess(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<Expression> subscriptExpression) :
        DoubleOperandExpression(std::move(postfixExpression), std::move(subscriptExpression), std::unique_ptr<Operator> { new Operator("[]") }) {
    // a[i] / p[i] is always an lvalue in C.
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
    return lvalue.get();
}

void ArrayAccess::setElementSize(int sizeInBytes) {
    elementSize = sizeInBytes;
}

int ArrayAccess::getElementSize() const {
    return elementSize;
}

bool ArrayAccess::baseIsArray() const {
    return baseIsArrayFlag;
}

void ArrayAccess::setBaseIsArray(bool value) {
    baseIsArrayFlag = value;
}

} // namespace ast
