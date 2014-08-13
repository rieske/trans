#include "LogicalOrExpression.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

const std::string LogicalOrExpression::ID { "<log_expr>" };

LogicalOrExpression::LogicalOrExpression(std::unique_ptr<Expression> leftHandSide,
        std::unique_ptr<Expression> rightHandSide) :
        LogicalExpression(std::move(leftHandSide), std::move(rightHandSide)) {
    saveExpressionAttributes(*this->leftHandSide);
    value = "rval";
}

void LogicalOrExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

}
