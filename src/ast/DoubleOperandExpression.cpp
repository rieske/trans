#include "DoubleOperandExpression.h"

#include <algorithm>
#include "Operator.h"

namespace ast {

DoubleOperandExpression::DoubleOperandExpression(std::unique_ptr<Expression> leftOperand, std::unique_ptr<Expression> rightOperand,
        std::unique_ptr<Operator> _operator) :
        leftOperand { std::move(leftOperand) }, rightOperand { std::move(rightOperand) }, _operator { std::move(_operator) } {
}

DoubleOperandExpression::~DoubleOperandExpression() {
}

const TranslationUnitContext& DoubleOperandExpression::getContext() const {
    return leftOperand->getContext();
}

Expression* DoubleOperandExpression::getLeftOperand() const {
    return leftOperand.get();
}

Expression* DoubleOperandExpression::getRightOperand() const {
    return rightOperand.get();
}

Operator* DoubleOperandExpression::getOperator() const {
    return _operator.get();
}

} /* namespace ast */
