#include "AssignmentExpression.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "Operator.h"

namespace ast {

const std::string AssignmentExpression::ID = "<a_expr>";

AssignmentExpression::AssignmentExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> assignmentOperator,
        std::unique_ptr<Expression> rightHandSide) :
        DoubleOperandExpression(std::move(leftHandSide), std::move(rightHandSide), std::move(assignmentOperator)) {
    lval = leftOperand->isLval();
}

void AssignmentExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

}

