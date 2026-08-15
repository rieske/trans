#include "DoubleOperandExpression.h"

#include "ParseEnvironment.h"
#include "types/Type.h"

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

bool DoubleOperandExpression::evaluateConstant(long& value) const {
    if (!_operator) {
        return false;
    }
    long left = 0;
    long right = 0;
    if (!leftOperand->evaluateConstant(left) || !rightOperand->evaluateConstant(right)) {
        return false;
    }
    const std::string op = _operator->getLexeme();
    if (op == "+") { value = left + right; return true; }
    if (op == "-") { value = left - right; return true; }
    if (op == "*") { value = left * right; return true; }
    if (op == "/") { if (right == 0) return false; value = left / right; return true; }
    if (op == "%") { if (right == 0) return false; value = left % right; return true; }
    if (op == "<<") { if (right < 0 || right >= 64) return false; value = left << right; return true; }
    if (op == ">>") { if (right < 0 || right >= 64) return false; value = left >> right; return true; }
    if (op == "&") { value = left & right; return true; }
    if (op == "|") { value = left | right; return true; }
    if (op == "^") { value = left ^ right; return true; }
    if (op == "<") { value = left < right; return true; }
    if (op == ">") { value = left > right; return true; }
    if (op == "<=") { value = left <= right; return true; }
    if (op == ">=") { value = left >= right; return true; }
    if (op == "==") { value = left == right; return true; }
    if (op == "!=") { value = left != right; return true; }
    if (op == "&&") { value = left && right; return true; }
    if (op == "||") { value = left || right; return true; }
    return false;
}

void DoubleOperandExpression::visitLeftOperand(AbstractSyntaxTreeVisitor& visitor) {
    leftOperand->accept(visitor);
}

void DoubleOperandExpression::visitRightOperand(AbstractSyntaxTreeVisitor& visitor) {
    rightOperand->accept(visitor);
}

type::Type DoubleOperandExpression::leftOperandType() const {
    return leftOperand->getType();
}

type::Type DoubleOperandExpression::rightOperandType() const {
    return rightOperand->getType();
}

bool DoubleOperandExpression::hasLeftOperandSymbol(const symbols::AnnotationStore& store) const {
    return leftOperand->hasResultSymbol(store);
}

bool DoubleOperandExpression::hasRightOperandSymbol(const symbols::AnnotationStore& store) const {
    return rightOperand->hasResultSymbol(store);
}

symbols::ValueEntry* DoubleOperandExpression::leftOperandSymbol(symbols::AnnotationStore& store) const {
    return leftOperand->getResultSymbol(store);
}

symbols::ValueEntry* DoubleOperandExpression::rightOperandSymbol(symbols::AnnotationStore& store) const {
    return rightOperand->getResultSymbol(store);
}

Expression* DoubleOperandExpression::getLeftOperand() const {
    return leftOperand.get();
}

Expression* DoubleOperandExpression::getRightOperand() const {
    return rightOperand.get();
}

} // namespace ast

