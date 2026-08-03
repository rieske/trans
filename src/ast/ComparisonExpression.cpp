#include "ComparisonExpression.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "Operator.h"
#include "types/Type.h"
#include "types/TypeQuery.h"

namespace ast {

ComparisonExpression::ComparisonExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> comparisonOperator,
        std::unique_ptr<Expression> rightHandSide)
    : DoubleOperandExpression(std::move(leftHandSide), std::move(rightHandSide), std::move(comparisonOperator)) {
    setType(type::signedInteger());
}

void ComparisonExpression::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }

bool ComparisonExpression::evaluateConstant(long& value) const {
    long left = 0;
    long right = 0;
    if (!leftOperand->evaluateConstant(left) || !rightOperand->evaluateConstant(right)) {
        return false;
    }
    auto unsignedIntegral = [](Expression* expr) {
        return expr->hasExpressionType() && type::isIntegral(expr->expressionType())
                && !type::valueIsSigned(expr->expressionType());
    };
    const bool unsignedCmp = unsignedIntegral(leftOperand.get()) || unsignedIntegral(rightOperand.get());
    const unsigned long ul = static_cast<unsigned long>(left);
    const unsigned long ur = static_cast<unsigned long>(right);
    switch (_operator->getKind()) {
    case OperatorKind::Lt: value = unsignedCmp ? ul < ur : left < right; return true;
    case OperatorKind::Gt: value = unsignedCmp ? ul > ur : left > right; return true;
    case OperatorKind::Le: value = unsignedCmp ? ul <= ur : left <= right; return true;
    case OperatorKind::Ge: value = unsignedCmp ? ul >= ur : left >= right; return true;
    case OperatorKind::Eq: value = left == right; return true;
    case OperatorKind::Ne: value = left != right; return true;
    default: return false;
    }
}

} // namespace ast
