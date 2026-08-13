#include "UnaryExpression.h"
#include "AbstractSyntaxTreeVisitor.h"
namespace ast {
UnaryExpression::UnaryExpression(std::unique_ptr<Operator> unaryOperator, std::unique_ptr<Expression> castExpression)
    : SingleOperandExpression(std::move(castExpression), std::move(unaryOperator)) {}
void UnaryExpression::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
bool UnaryExpression::isLval() const {
    return getOperator()->getKind() == OperatorKind::Deref;
}
bool UnaryExpression::evaluateConstant(long& value) const {
    const OperatorKind op = getOperator()->getKind();
    if ((op == OperatorKind::Sizeof || op == OperatorKind::AddressOf) && foldedInteger_) {
        value = *foldedInteger_;
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
void UnaryExpression::setFoldedInteger(long value) { foldedInteger_ = value; }
bool UnaryExpression::hasFoldedInteger() const { return foldedInteger_.has_value(); }
long UnaryExpression::foldedInteger() const { return *foldedInteger_; }
bool UnaryExpression::isSizeof() const { return getOperator()->getKind() == OperatorKind::Sizeof; }
} // namespace ast
