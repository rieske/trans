#include "SingleOperandExpression.h"

#include <algorithm>
#include "Operator.h"

namespace ast {

SingleOperandExpression::SingleOperandExpression(std::unique_ptr<Expression> _operand, std::unique_ptr<Operator> _operator) :
        _operand { std::move(_operand) },
        _operator { std::move(_operator) }
{
}

SingleOperandExpression::~SingleOperandExpression() {
}

void SingleOperandExpression::visitOperand(AbstractSyntaxTreeVisitor& visitor) {
    _operand->accept(visitor);
}

const FundamentalType& SingleOperandExpression::operandType() const {
    return _operand->getType();
}

semantic_analyzer::ValueEntry* SingleOperandExpression::operandSymbol() const {
    return _operand->getResultSymbol();
}

translation_unit::Context SingleOperandExpression::getContext() const {
    return _operand->getContext();
}

bool SingleOperandExpression::isLval() const {
    return _operand->isLval();
}

Operator* SingleOperandExpression::getOperator() const {
    return _operator.get();
}

} /* namespace ast */

