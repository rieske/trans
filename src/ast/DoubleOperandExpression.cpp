#include "DoubleOperandExpression.h"

#include "ParseEnvironment.h"
#include "types/IntegerConstant.h"
#include "types/Type.h"
#include "types/TypeQuery.h"

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

std::optional<type::Type> DoubleOperandExpression::intIfOperandsType(
        const ParseEnvironment& environment) const {
    if (!leftOperand->typeAtParseTime(environment) || !rightOperand->typeAtParseTime(environment)) {
        return std::nullopt;
    }
    return type::signedInteger();
}

translation_unit::Context DoubleOperandExpression::getContext() const {
    return leftOperand->getContext();
}

Operator* DoubleOperandExpression::getOperator() const {
    return _operator.get();
}

bool DoubleOperandExpression::evaluateConstant(type::IntegerConstant& value) const {
    if (!_operator) {
        return false;
    }
    type::IntegerConstant left;
    type::IntegerConstant right;
    if (!leftOperand->evaluateConstant(left) || !rightOperand->evaluateConstant(right)) {
        return false;
    }
    auto folded = type::foldBinary(_operator->getLexeme(), left, right);
    if (!folded) {
        return false;
    }
    value = *folded;
    return true;
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

} // namespace ast
