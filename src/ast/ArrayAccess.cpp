#include "ArrayAccess.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "Operator.h"
#include "ParseEnvironment.h"
#include "types/TypeQuery.h"

namespace ast {

ArrayAccess::ArrayAccess(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<Expression> subscriptExpression)
    : DoubleOperandExpression(std::move(postfixExpression), std::move(subscriptExpression), std::unique_ptr<Operator>{new Operator("[]")}) {
    lval = true;
}

void ArrayAccess::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

std::optional<type::Type> ArrayAccess::typeAtParseTime(const ParseEnvironment& environment) const {
    auto left = leftOperand->typeAtParseTime(environment);
    if (left) {
        if (auto element = type::afterLvalueConversion(*left).indexElement()) {
            return element;
        }
    }
    // C 6.5.2.1: E1[E2] is *(E1+E2), so the pointer may be on either side (i[p]).
    auto right = rightOperand->typeAtParseTime(environment);
    if (!right) {
        return std::nullopt;
    }
    return type::afterLvalueConversion(*right).indexElement();
}

} // namespace ast
