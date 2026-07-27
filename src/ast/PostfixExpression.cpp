#include "PostfixExpression.h"

#include <cassert>

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

PostfixExpression::PostfixExpression(std::unique_ptr<Expression> postfixExpression,
        std::unique_ptr<Operator> postfixOperator) :
        SingleOperandExpression { std::move(postfixExpression), std::move(postfixOperator) } {
}

void PostfixExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void PostfixExpression::setPreOperationSymbol(symbols::AnnotationStore& store, symbols::ValueEntry resultSymbol) {
    setType(resultSymbol.getType());
    store.setPreOperation(this, std::move(resultSymbol));
}

symbols::ValueEntry* PostfixExpression::getPreOperationSymbol(symbols::AnnotationStore& store) const {
    auto* pre = store.preOperation(this);
    assert(pre);
    return pre;
}

} // namespace ast
