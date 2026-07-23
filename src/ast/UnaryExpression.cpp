#include "UnaryExpression.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "symbols/AnnotationStore.h"
namespace ast {
UnaryExpression::UnaryExpression(std::unique_ptr<Operator> unaryOperator, std::unique_ptr<Expression> castExpression)
    : SingleOperandExpression(std::move(castExpression), std::move(unaryOperator)) {}
void UnaryExpression::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
bool UnaryExpression::isLval() const {
    return getOperator()->getKind() == OperatorKind::Deref;
}
bool UnaryExpression::evaluateConstant(long& value) const {
    const OperatorKind op = getOperator()->getKind();
    if ((op == OperatorKind::Sizeof || op == OperatorKind::AddressOf) && sizeofValue >= 0) {
        value = sizeofValue;
        return true;
    }
    if (op == OperatorKind::AddressOf) return false;
    long operand = 0;
    if (!_operand->evaluateConstant(operand)) return false;
    if (op == OperatorKind::Sub) { value = -operand; return true; }
    if (op == OperatorKind::Add) { value = operand; return true; }
    if (op == OperatorKind::LogicalNot) { value = !operand; return true; }
    if (op == OperatorKind::BitNot) { value = ~operand; return true; }
    return false;
}
void UnaryExpression::setTruthyLabel(symbols::LabelEntry truthyLabel) {
    symbols::AnnotationStore::current().setLabel(this, symbols::LabelSlot::Truthy, std::move(truthyLabel));
}
symbols::LabelEntry* UnaryExpression::getTruthyLabel() const {
    return symbols::AnnotationStore::current().label(this, symbols::LabelSlot::Truthy);
}
void UnaryExpression::setFalsyLabel(symbols::LabelEntry falsyLabel) {
    symbols::AnnotationStore::current().setLabel(this, symbols::LabelSlot::Falsy, std::move(falsyLabel));
}
symbols::LabelEntry* UnaryExpression::getFalsyLabel() const {
    return symbols::AnnotationStore::current().label(this, symbols::LabelSlot::Falsy);
}
void UnaryExpression::setLvalueSymbol(symbols::ValueEntry lvalueSymbol) {
    symbols::AnnotationStore::current().setValue(this, symbols::ValueSlot::Lvalue, std::move(lvalueSymbol));
}
symbols::ValueEntry* UnaryExpression::getLvalueSymbol() const {
    return symbols::AnnotationStore::current().value(this, symbols::ValueSlot::Lvalue);
}
void UnaryExpression::setSizeofValue(int bytes) { sizeofValue = bytes; }
int UnaryExpression::getSizeofValue() const { return sizeofValue; }
bool UnaryExpression::isSizeof() const { return getOperator()->getKind() == OperatorKind::Sizeof; }
} // namespace ast
