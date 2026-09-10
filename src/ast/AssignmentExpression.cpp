#include "AssignmentExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "ParseEnvironment.h"
#include "types/TypeQuery.h"

namespace ast {

AssignmentExpression::AssignmentExpression(std::unique_ptr<Expression> leftHandSide,
        type::AssignOp op,
        std::unique_ptr<Expression> rightHandSide) :
        BinaryOpExpression(std::move(leftHandSide), op, std::move(rightHandSide))
{
}

void AssignmentExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

std::optional<type::Type> AssignmentExpression::typeAtParseTime(const ParseEnvironment& environment) const {
    auto left = leftOperand->typeAtParseTime(environment);
    if (!left || !rightOperand->typeAtParseTime(environment)) {
        return std::nullopt;
    }
    return type::afterLvalueConversion(*left);
}

symbols::ValueEntry* AssignmentExpression::leftOperandLvalueSymbol(symbols::AnnotationStore& store) const {
    return leftOperand->getLvalueSymbol(store);
}

} // namespace ast

