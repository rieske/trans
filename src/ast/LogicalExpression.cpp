#include "LogicalExpression.h"
#include "types/Type.h"
namespace ast {
LogicalExpression::LogicalExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> logicalOperator,
        std::unique_ptr<Expression> rightHandSide)
    : DoubleOperandExpression{std::move(leftHandSide), std::move(rightHandSide), std::move(logicalOperator)} {
    setType(type::signedInteger());
}
LogicalExpression::~LogicalExpression() = default;
} // namespace ast
