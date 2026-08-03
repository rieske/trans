#include "PostfixExpression.h"
#include "AbstractSyntaxTreeVisitor.h"
namespace ast {
PostfixExpression::PostfixExpression(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<Operator> postfixOperator)
    : SingleOperandExpression(std::move(postfixExpression), std::move(postfixOperator)) {}
void PostfixExpression::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
} // namespace ast
