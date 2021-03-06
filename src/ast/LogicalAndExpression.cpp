#include "LogicalAndExpression.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

LogicalAndExpression::LogicalAndExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Expression> rightHandSide) :
        LogicalExpression(std::move(leftHandSide), std::unique_ptr<Operator> { new Operator("&&") }, std::move(rightHandSide)) {
}

void LogicalAndExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace ast

