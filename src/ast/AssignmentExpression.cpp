#include "AssignmentExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "Operator.h"
#include "ParseEnvironment.h"
#include "types/TypeQuery.h"

namespace ast {

AssignmentExpression::AssignmentExpression(std::unique_ptr<Expression> leftHandSide,
        std::unique_ptr<Operator> assignmentOperator,
        std::unique_ptr<Expression> rightHandSide) :
        DoubleOperandExpression(std::move(leftHandSide), std::move(rightHandSide), std::move(assignmentOperator))
{
}

void AssignmentExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

std::optional<type::Type> AssignmentExpression::typeAtParseTime(const ParseEnvironment& environment) const {
    auto left = leftOperand->typeAtParseTime(environment);
    if (!left || !rightOperand->typeAtParseTime(environment)) {
        return std::nullopt;
    }
    return type::afterLvalueConversion(*left);
}

bool AssignmentExpression::isLval() const {
    return false;
}

} // namespace ast
