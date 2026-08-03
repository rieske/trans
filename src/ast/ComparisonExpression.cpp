#include "ComparisonExpression.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "Operator.h"
#include "types/Type.h"
namespace ast {
ComparisonExpression::ComparisonExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> comparisonOperator,
        std::unique_ptr<Expression> rightHandSide)
    : DoubleOperandExpression(std::move(leftHandSide), std::move(rightHandSide), std::move(comparisonOperator)) {
    setType(type::signedInteger());
}
void ComparisonExpression::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
} // namespace ast
