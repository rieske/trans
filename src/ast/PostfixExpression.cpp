#include "PostfixExpression.h"

#include <cassert>

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

PostfixExpression::PostfixExpression(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<Operator> postfixOperator) :
        SingleOperandExpression(std::move(postfixExpression), std::move(postfixOperator)) {
}

void PostfixExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void PostfixExpression::setPreOperationSymbol(symbols::ValueEntry resultSymbol) {
    this->preOperationSymbol = std::make_unique<symbols::ValueEntry>(resultSymbol);
    setType(this->preOperationSymbol->getType());
}

symbols::ValueEntry* PostfixExpression::getPreOperationSymbol() const {
    assert(preOperationSymbol);
    return preOperationSymbol.get();
}

} // namespace ast

