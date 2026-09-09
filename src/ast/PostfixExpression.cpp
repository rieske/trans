#include "PostfixExpression.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

PostfixExpression::PostfixExpression(std::unique_ptr<Expression> postfixExpression,
        type::IncDec op) :
        UnaryOpExpression { std::move(postfixExpression), op } {
}

void PostfixExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace ast
