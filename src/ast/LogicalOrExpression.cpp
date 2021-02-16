#include "LogicalOrExpression.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

const std::string LogicalOrExpression::ID { "<logical_or_exp>" };

LogicalOrExpression::LogicalOrExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Expression> rightHandSide) :
        LogicalExpression(std::move(leftHandSide), std::unique_ptr<Operator> { new Operator("||") }, std::move(rightHandSide)) {
}

void LogicalOrExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace ast

