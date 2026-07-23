#include "SingleOperandExpression.h"

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

type::Type SingleOperandExpression::operandType() const {
    return _operand->expressionType();
}

type::Type SingleOperandExpression::operandValueType() const {
    return _operand->valueType();
}

bool SingleOperandExpression::hasOperandSymbol() const {
    return _operand->hasResultSymbol();
}

symbols::ValueEntry* SingleOperandExpression::operandSymbol() const {
    return _operand->getResultSymbol();
}

symbols::ValueEntry* SingleOperandExpression::operandLvalueSymbol() const {
    return _operand->getLvalueSymbol();
}

Expression* SingleOperandExpression::getOperandExpression() const {
    return _operand.get();
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

} // namespace ast

