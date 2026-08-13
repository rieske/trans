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
    return getOperator()->getLexeme() == "*" || lval;
}

void UnaryExpression::setLval(bool value) {
    lval = value;
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
    if (op == "~") { value = ~operand; return true; }
    return false;
}

void UnaryExpression::setTruthyLabel(symbols::AnnotationStore& store, symbols::LabelEntry truthyLabel) {
    store.setLabel(this, symbols::LabelSlot::Truthy, std::move(truthyLabel));
}

symbols::LabelEntry* UnaryExpression::getTruthyLabel(symbols::AnnotationStore& store) const {
    return store.label(this, symbols::LabelSlot::Truthy);
}

void UnaryExpression::setFalsyLabel(symbols::AnnotationStore& store, symbols::LabelEntry falsyLabel) {
    store.setLabel(this, symbols::LabelSlot::Falsy, std::move(falsyLabel));
}

symbols::LabelEntry* UnaryExpression::getFalsyLabel(symbols::AnnotationStore& store) const {
    return store.label(this, symbols::LabelSlot::Falsy);
}


} // namespace ast

