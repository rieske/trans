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

bool DoubleOperandExpression::evaluateConstant(long& value) const {
    if (!_operator) {
        return false;
    }
    long left = 0;
    long right = 0;
    if (!leftOperand->evaluateConstant(left) || !rightOperand->evaluateConstant(right)) {
        return false;
    }
    switch (_operator->getKind()) {
    case OperatorKind::Add: value = left + right; return true;
    case OperatorKind::Sub: value = left - right; return true;
    case OperatorKind::Mul: value = left * right; return true;
    case OperatorKind::Div:
        if (right == 0) return false;
        value = left / right;
        return true;
    case OperatorKind::Mod:
        if (right == 0) return false;
        value = left % right;
        return true;
    case OperatorKind::Shl:
        if (right < 0 || right >= 64) return false;
        value = left << right;
        return true;
    case OperatorKind::Shr:
        if (right < 0 || right >= 64) return false;
        value = left >> right;
        return true;
    case OperatorKind::BitAnd: value = left & right; return true;
    case OperatorKind::BitOr: value = left | right; return true;
    case OperatorKind::BitXor: value = left ^ right; return true;
    case OperatorKind::Lt: value = left < right; return true;
    case OperatorKind::Gt: value = left > right; return true;
    case OperatorKind::Le: value = left <= right; return true;
    case OperatorKind::Ge: value = left >= right; return true;
    case OperatorKind::Eq: value = left == right; return true;
    case OperatorKind::Ne: value = left != right; return true;
    case OperatorKind::LogicalAnd: value = left && right; return true;
    case OperatorKind::LogicalOr: value = left || right; return true;
    default: return false;
    }
}

void DoubleOperandExpression::visitLeftOperand(AbstractSyntaxTreeVisitor& visitor) {
    leftOperand->accept(visitor);
}

void DoubleOperandExpression::visitRightOperand(AbstractSyntaxTreeVisitor& visitor) {
    rightOperand->accept(visitor);
}

Expression* DoubleOperandExpression::getLeftOperand() const {
    return leftOperand.get();
}

Expression* DoubleOperandExpression::getRightOperand() const {
    return rightOperand.get();
}

type::Type DoubleOperandExpression::leftOperandType() const {
    return leftOperand->expressionType();
}

type::Type DoubleOperandExpression::rightOperandType() const {
    return rightOperand->expressionType();
}

type::Type DoubleOperandExpression::leftValueType() const {
    return leftOperand->valueType();
}

type::Type DoubleOperandExpression::rightValueType() const {
    return rightOperand->valueType();
}

bool DoubleOperandExpression::hasLeftOperandSymbol() const {
    return leftOperand->hasResultSymbol();
}

bool DoubleOperandExpression::hasRightOperandSymbol() const {
    return rightOperand->hasResultSymbol();
}

symbols::ValueEntry* DoubleOperandExpression::leftOperandSymbol() const {
    return leftOperand->getResultSymbol();
}

symbols::ValueEntry* DoubleOperandExpression::rightOperandSymbol() const {
    return rightOperand->getResultSymbol();
}

} // namespace ast

