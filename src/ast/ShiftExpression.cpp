#include "ShiftExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "ParseEnvironment.h"
#include "types/TypeQuery.h"

namespace ast {

ShiftExpression::ShiftExpression(std::unique_ptr<Expression> shiftExpression, std::unique_ptr<Operator> shiftOperator,
        std::unique_ptr<Expression> additionExpression) :
        DoubleOperandExpression { std::move(shiftExpression), std::move(additionExpression), std::move(shiftOperator) } {
}

void ShiftExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

std::optional<type::Type> ShiftExpression::typeAtParseTime(const ParseEnvironment& environment) const {
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
    return type::integerPromote(leftConv);
}

} // namespace ast

