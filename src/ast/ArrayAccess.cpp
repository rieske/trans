#include "ArrayAccess.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "Operator.h"

namespace ast {

ArrayAccess::ArrayAccess(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<Expression> subscriptExpression)
    : DoubleOperandExpression(std::move(postfixExpression), std::move(subscriptExpression), std::unique_ptr<Operator>{new Operator("[]")}) {
    lval = true;
}

void ArrayAccess::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace ast
