#include "LogicalAndExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "types/IntegerConstant.h"
#include "types/Operator.h"

namespace ast {

LogicalAndExpression::LogicalAndExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Expression> rightHandSide) :
        LogicalExpression(std::move(leftHandSide), std::move(rightHandSide)) {
}

void LogicalAndExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

bool LogicalAndExpression::evaluateConstant(type::IntegerConstant& value) const {
    return foldOperands(value, type::BinaryOp::LogAnd);
}

} // namespace ast

