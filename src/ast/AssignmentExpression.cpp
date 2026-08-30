#include "AssignmentExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "ParseEnvironment.h"
#include "types/TypeQuery.h"

namespace ast {

AssignmentExpression::AssignmentExpression(std::unique_ptr<Expression> leftHandSide,
        std::string lexeme,
        std::unique_ptr<Expression> rightHandSide) :
        BinaryOpExpression(std::move(leftHandSide), std::move(lexeme), std::move(rightHandSide))
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

bool AssignmentExpression::isLval() const {
    // C: an assignment expression is never an lvalue. SA checks leftOperand->isLval()
    // for the assignment constraint separately.
    return false;
}

symbols::ValueEntry* AssignmentExpression::leftOperandLvalueSymbol(symbols::AnnotationStore& store) const {
    return leftOperand->getLvalueSymbol(store);
}

} // namespace ast

