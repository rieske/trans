#include "LogicalOrExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "types/IntegerConstant.h"

namespace ast {

LogicalOrExpression::LogicalOrExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Expression> rightHandSide) :
        LogicalExpression(std::move(leftHandSide), std::move(rightHandSide)) {
}

void LogicalOrExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

bool LogicalOrExpression::evaluateConstant(type::IntegerConstant& value) const {
    return foldOperands(value, "||");
}

} // namespace ast

