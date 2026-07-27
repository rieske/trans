#include "ArrayAccess.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

ArrayAccess::ArrayAccess(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<Expression> subscriptExpression) :
        DoubleOperandExpression(std::move(postfixExpression), std::move(subscriptExpression),
                std::make_unique<Operator>("[]"))
{
    // Element access is always an lvalue when the base is addressable (array or pointer).
    lval = true;
}

void ArrayAccess::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void ArrayAccess::setLvalue(symbols::AnnotationStore& store, symbols::ValueEntry lvalue) {
    setLvalueSymbol(store, std::move(lvalue));
}

symbols::ValueEntry* ArrayAccess::getLvalue(symbols::AnnotationStore& store) const {
    return getLvalueSymbol(store);
}

symbols::ValueEntry* ArrayAccess::getLvalueSymbol(symbols::AnnotationStore& store) const {
    return store.lvalue(this);
}

void ArrayAccess::setElementSize(int sizeInBytes) {
    elementSize = sizeInBytes;
}

int ArrayAccess::getElementSize() const {
    return elementSize;
}

} // namespace ast
