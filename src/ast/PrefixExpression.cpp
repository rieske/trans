#include "PrefixExpression.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

PrefixExpression::PrefixExpression(type::IncDec op, std::unique_ptr<Expression> unaryExpression) :
        UnaryOpExpression(std::move(unaryExpression), op) {
}

void PrefixExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace ast

