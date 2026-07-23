#include "UnaryExpression.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

UnaryExpression::UnaryExpression(std::unique_ptr<Operator> unaryOperator, std::unique_ptr<Expression> castExpression) :
        SingleOperandExpression(std::move(castExpression), std::move(unaryOperator))
{
}

void UnaryExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

bool UnaryExpression::isLval() const {
    // Only dereference yields an lvalue; +a, -a, !a, &a are rvalues.
    return getOperator()->getKind() == OperatorKind::Deref;
}

bool UnaryExpression::evaluateConstant(long& value) const {
    const OperatorKind op = getOperator()->getKind();
    // Folded sizeof / offsetof (set during semantic analysis).
    if ((op == OperatorKind::Sizeof || op == OperatorKind::AddressOf) && sizeofValue >= 0) {
        value = sizeofValue;
        return true;
    }
    if (op == OperatorKind::AddressOf) {
        return false;
    }
    long operand = 0;
    if (!_operand->evaluateConstant(operand)) {
        return false;
    }
    if (op == OperatorKind::Sub) { value = -operand; return true; }
    if (op == OperatorKind::Add) { value = operand; return true; }
    if (op == OperatorKind::LogicalNot) { value = !operand; return true; }
    if (op == OperatorKind::BitNot) { value = ~operand; return true; }
    return false;
}

void UnaryExpression::setTruthyLabel(semantic_analyzer::LabelEntry truthyLabel) {
    this->truthyLabel = std::unique_ptr<semantic_analyzer::LabelEntry> {
            new semantic_analyzer::LabelEntry { truthyLabel } };
}

semantic_analyzer::LabelEntry* UnaryExpression::getTruthyLabel() const {
    return truthyLabel.get();
}

void UnaryExpression::setFalsyLabel(semantic_analyzer::LabelEntry falsyLabel) {
    this->falsyLabel =
            std::unique_ptr<semantic_analyzer::LabelEntry> { new semantic_analyzer::LabelEntry { falsyLabel } };
}

semantic_analyzer::LabelEntry* UnaryExpression::getFalsyLabel() const {
    return falsyLabel.get();
}

void UnaryExpression::setLvalueSymbol(semantic_analyzer::ValueEntry lvalueSymbol) {
    this->lvalueSymbol = std::make_unique<semantic_analyzer::ValueEntry>(lvalueSymbol);
}

semantic_analyzer::ValueEntry* UnaryExpression::getLvalueSymbol() const {
    return lvalueSymbol.get();
}

void UnaryExpression::setSizeofValue(int bytes) {
    sizeofValue = bytes;
}

int UnaryExpression::getSizeofValue() const {
    return sizeofValue;
}

bool UnaryExpression::isSizeof() const {
    return getOperator()->getKind() == OperatorKind::Sizeof;
}

} // namespace ast

