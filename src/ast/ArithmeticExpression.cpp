#include "ArithmeticExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "Operator.h"
#include "ParseEnvironment.h"
#include "types/TypeQuery.h"

namespace ast {

ArithmeticExpression::ArithmeticExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> arithmeticOperator,
        std::unique_ptr<Expression> rightHandSide) :
        DoubleOperandExpression(std::move(leftHandSide), std::move(rightHandSide), std::move(arithmeticOperator)) {
}

void ArithmeticExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

std::optional<type::Type> ArithmeticExpression::typeAtParseTime(const ParseEnvironment& environment) const {
    auto left = leftOperand->typeAtParseTime(environment);
    auto right = rightOperand->typeAtParseTime(environment);
    if (!left || !right) {
        return std::nullopt;
    }
    return type::arithmeticExpressionResult(*left, *right, getOperator()->getLexeme().front());
}

} // namespace ast
