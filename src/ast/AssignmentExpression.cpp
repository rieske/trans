#include "AssignmentExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "Operator.h"

namespace ast {

AssignmentExpression::AssignmentExpression(std::unique_ptr<Expression> leftHandSide,
        std::unique_ptr<Operator> assignmentOperator,
        std::unique_ptr<Expression> rightHandSide) :
        DoubleOperandExpression(std::move(leftHandSide), std::move(rightHandSide), std::move(assignmentOperator))
{
    lval = leftOperand->isLval();
}

void AssignmentExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

semantic_analyzer::ValueEntry* AssignmentExpression::leftOperandLvalueSymbol() const {
    return leftOperand->getLvalueSymbol();
}

void AssignmentExpression::setPointerArithmetic(int scale, std::string scaleTemp) {
    pointerScale = scale;
    scaleTempName = std::move(scaleTemp);
}

int AssignmentExpression::getPointerScale() const {
    return pointerScale;
}

const std::string* AssignmentExpression::getScaleTempName() const {
    return scaleTempName ? &*scaleTempName : nullptr;
}

} // namespace ast
