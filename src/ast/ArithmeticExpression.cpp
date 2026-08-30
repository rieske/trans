#include "ArithmeticExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "ParseEnvironment.h"
#include "types/TypeQuery.h"

namespace ast {

ArithmeticExpression::ArithmeticExpression(std::unique_ptr<Expression> leftHandSide, std::string lexeme,
        std::unique_ptr<Expression> rightHandSide) :
        BinaryOpExpression(std::move(leftHandSide), std::move(lexeme), std::move(rightHandSide)) {
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
    return type::arithmeticExpressionResult(*left, *right, lexeme().front());
}

} // namespace ast
