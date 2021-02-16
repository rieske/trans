#include "DoubleOperandExpression.h"

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

const FundamentalType& DoubleOperandExpression::leftOperandType() const {
    return leftOperand->getType();
}

const FundamentalType& DoubleOperandExpression::rightOperandType() const {
    return rightOperand->getType();
}

semantic_analyzer::ValueEntry* DoubleOperandExpression::leftOperandSymbol() const {
    return leftOperand->getResultSymbol();
}

semantic_analyzer::ValueEntry* DoubleOperandExpression::rightOperandSymbol() const {
    return rightOperand->getResultSymbol();
}

} // namespace ast

