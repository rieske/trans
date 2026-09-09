#include "ExpressionList.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "ParseEnvironment.h"
#include "types/TypeQuery.h"

namespace ast {

ExpressionList::ExpressionList(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Expression> rightHandSide) :
        DoubleOperandExpression(std::move(leftHandSide), std::move(rightHandSide)) {
}

ExpressionList::~ExpressionList() {
}

void ExpressionList::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

std::optional<type::Type> ExpressionList::typeAtParseTime(const ParseEnvironment& environment) const {
    if (!leftOperand->typeAtParseTime(environment)) {
        return std::nullopt;
    }
    const std::optional<type::Type> right = rightOperand->typeAtParseTime(environment);
    return right ? type::afterLvalueConversion(*right) : right;
}

} // namespace ast

