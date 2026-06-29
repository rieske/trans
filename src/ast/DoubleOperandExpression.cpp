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

semantic_analyzer::ValueEntry* DoubleOperandExpression::leftOperandSymbol() const {
    return leftOperand->getResultSymbol();
}

semantic_analyzer::ValueEntry* DoubleOperandExpression::rightOperandSymbol() const {
    return rightOperand->getResultSymbol();
}

} // namespace ast

