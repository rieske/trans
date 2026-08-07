#include "FunctionCall.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

FunctionCall::FunctionCall(std::unique_ptr<Expression> postfixExpression,
        std::vector<std::unique_ptr<Expression>> argumentList) :
        SingleOperandExpression { std::move(postfixExpression), std::unique_ptr<Operator> { new Operator("()") } },
        argumentList { std::move(argumentList) }
{
}

void FunctionCall::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void FunctionCall::visitArguments(AbstractSyntaxTreeVisitor& visitor) {
    for (const auto& argument : argumentList) {
        argument->accept(visitor);
    }
}

const std::vector<std::unique_ptr<Expression>>& FunctionCall::getArgumentList() const {
    return argumentList;
}

void FunctionCall::setBuiltinTypeArgument(type::Type type) {
    builtinTypeArgument_ = std::move(type);
}

const type::Type* FunctionCall::builtinTypeArgument() const {
    return builtinTypeArgument_ ? &*builtinTypeArgument_ : nullptr;
}

} // namespace ast
