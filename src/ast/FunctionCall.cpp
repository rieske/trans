#include "FunctionCall.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

FunctionCall::FunctionCall(std::unique_ptr<Expression> callExpression,
        std::vector<std::unique_ptr<Expression>> argumentList)
    : SingleOperandExpression(std::move(callExpression), nullptr),
      argumentList{std::move(argumentList)} {}

void FunctionCall::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }

void FunctionCall::visitArguments(AbstractSyntaxTreeVisitor& visitor) {
    for (auto& arg : argumentList) {
        arg->accept(visitor);
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
