#include "BitwiseExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "Operator.h"
#include "ParseEnvironment.h"
#include "types/TypeQuery.h"

namespace ast {

BitwiseExpression::BitwiseExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> bitwiseOperator,
        std::unique_ptr<Expression> rightHandSide) :
        DoubleOperandExpression(std::move(leftHandSide), std::move(rightHandSide), std::move(bitwiseOperator)) {
}

void BitwiseExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

std::optional<type::Type> BitwiseExpression::typeAtParseTime(const ParseEnvironment& environment) const {
    auto left = leftOperand->typeAtParseTime(environment);
    auto right = rightOperand->typeAtParseTime(environment);
    if (!left || !right) {
        return std::nullopt;
    }
    const type::Type leftConv = type::afterLvalueConversion(*left);
    const type::Type rightConv = type::afterLvalueConversion(*right);
    if (!type::isIntegral(leftConv) || !type::isIntegral(rightConv)) {
        return std::nullopt;
    }
    return type::usualArithmeticResult(leftConv, rightConv);
}

} // namespace ast
