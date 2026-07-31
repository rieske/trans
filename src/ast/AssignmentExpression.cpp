#include "AssignmentExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "Operator.h"

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

bool AssignmentExpression::isLval() const {
    // C: an assignment expression is never an lvalue. SA checks leftOperand->isLval()
    // for the assignment constraint separately.
    return false;
}

symbols::ValueEntry* AssignmentExpression::leftOperandLvalueSymbol(symbols::AnnotationStore& store) const {
    return leftOperand->getLvalueSymbol(store);
}

} // namespace ast

