#include "ArrayAccess.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

ArrayAccess::ArrayAccess(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<Expression> subscriptExpression) :
        DoubleOperandExpression(std::move(postfixExpression), std::move(subscriptExpression),
                std::make_unique<Operator>("[]"))
{
    lval = true;
}

void ArrayAccess::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void ArrayAccess::setElementSize(int sizeInBytes) {
    elementSize = sizeInBytes;
}

int ArrayAccess::getElementSize() const {
    return elementSize;
}

} // namespace ast
