#include "BitwiseExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "Operator.h"

namespace ast {

BitwiseExpression::BitwiseExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> bitwiseOperator,
        std::unique_ptr<Expression> rightHandSide) :
        DoubleOperandExpression(std::move(leftHandSide), std::move(rightHandSide), std::move(bitwiseOperator)) {
}

void BitwiseExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace ast
