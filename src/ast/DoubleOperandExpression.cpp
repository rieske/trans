#include "DoubleOperandExpression.h"

#include <algorithm>
#include "Operator.h"

namespace ast {

DoubleOperandExpression::DoubleOperandExpression(std::unique_ptr<Expression> leftOperand, std::unique_ptr<Expression> rightOperand,
        std::unique_ptr<Operator> _operator) :
        leftOperand { std::move(leftOperand) },
        rightOperand { std::move(rightOperand) },
        _operator { std::move(_operator) }
{
}

DoubleOperandExpression::~DoubleOperandExpression() {
}

translation_unit::Context DoubleOperandExpression::getContext() const {
    return leftOperand->getContext();
}

Operator* DoubleOperandExpression::getOperator() const {
    return _operator.get();
}

void DoubleOperandExpression::visitLeftOperand(AbstractSyntaxTreeVisitor& visitor) {
    leftOperand->accept(visitor);
}

void DoubleOperandExpression::visitRightOperand(AbstractSyntaxTreeVisitor& visitor) {
    rightOperand->accept(visitor);
}

Type DoubleOperandExpression::getLeftOperandType() const {
    return leftOperand->getType();
}

Type DoubleOperandExpression::getRightOperandType() const {
    return rightOperand->getType();
}

code_generator::ValueEntry* DoubleOperandExpression::getLeftOperandSymbol() const {
    return leftOperand->getResultSymbol();
}

code_generator::ValueEntry* DoubleOperandExpression::getRightOperandSymbol() const {
    return rightOperand->getResultSymbol();
}

} /* namespace ast */
