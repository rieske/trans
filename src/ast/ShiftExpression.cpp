#include "ShiftExpression.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

ShiftExpression::ShiftExpression(std::unique_ptr<Expression> shiftExpression, std::unique_ptr<Operator> shiftOperator,
        std::unique_ptr<Expression> additionExpression) :
        DoubleOperandExpression { std::move(shiftExpression), std::move(additionExpression), std::move(shiftOperator) } {
}

void ShiftExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace ast

