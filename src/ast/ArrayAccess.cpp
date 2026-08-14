#include "ArrayAccess.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "ParseEnvironment.h"
#include "types/TypeQuery.h"

namespace ast {

ArrayAccess::ArrayAccess(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<Expression> subscriptExpression) :
        DoubleOperandExpression(std::move(postfixExpression), std::move(subscriptExpression),
                std::make_unique<Operator>("[]"))
{
    lval = true;
}

void ArrayAccess::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

std::optional<type::Type> ArrayAccess::typeAtParseTime(const ParseEnvironment& environment) const {
    auto base = leftOperand->typeAtParseTime(environment);
    if (!base) {
        return std::nullopt;
    }
    return type::afterLvalueConversion(*base).indexElement();
}

} // namespace ast
