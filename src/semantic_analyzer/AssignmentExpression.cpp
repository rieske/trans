#include "AssignmentExpression.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

const std::string AssignmentExpression::ID = "<a_expr>";

AssignmentExpression::AssignmentExpression(std::unique_ptr<Expression> leftHandSide, TerminalSymbol assignmentOperator,
        std::unique_ptr<Expression> rightHandSide) :
        leftHandSide { std::move(leftHandSide) },
        assignmentOperator { assignmentOperator },
        rightHandSide { std::move(rightHandSide) } {
    saveExpressionAttributes(*this->leftHandSide);
}

void AssignmentExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

}

