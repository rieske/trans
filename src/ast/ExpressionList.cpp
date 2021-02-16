#include "ExpressionList.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

ExpressionList::ExpressionList(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Expression> rightHandSide) :
        DoubleOperandExpression(std::move(leftHandSide), std::move(rightHandSide), std::unique_ptr<Operator> { new Operator { "," } }) {
    lval = leftOperand->isLval();
}

ExpressionList::~ExpressionList() {
}

void ExpressionList::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace ast

