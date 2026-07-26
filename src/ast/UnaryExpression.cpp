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
    return getOperator()->getLexeme() == "*";
}

void UnaryExpression::setSizeofValue(int bytes) {
    sizeofValue = bytes;
}

int UnaryExpression::getSizeofValue() const {
    return sizeofValue;
}

bool UnaryExpression::evaluateConstant(long& value) const {
    if (getOperator()->getLexeme() == "sizeof" && sizeofValue >= 0) {
        value = sizeofValue;
        return true;
    }
    long operand = 0;
    if (!_operand->evaluateConstant(operand)) {
        return false;
    }
    const std::string op = getOperator()->getLexeme();
    if (op == "-") { value = -operand; return true; }
    if (op == "+") { value = operand; return true; }
    if (op == "!") { value = !operand; return true; }
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

void UnaryExpression::setLvalueSymbol(symbols::ValueEntry lvalueSymbol) {
    this->lvalueSymbol = std::make_unique<symbols::ValueEntry>(lvalueSymbol);
}

symbols::ValueEntry* UnaryExpression::getLvalueSymbol(symbols::AnnotationStore& store) const {
    (void)store;
    return lvalueSymbol.get();
}

} // namespace ast

