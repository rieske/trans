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

void FunctionCall::setBuiltinTypeName(TypeName name) {
    builtinTypeName_ = std::move(name);
}

TypeName* FunctionCall::builtinTypeName() {
    return builtinTypeName_ ? &*builtinTypeName_ : nullptr;
}

const TypeName* FunctionCall::builtinTypeName() const {
    return builtinTypeName_ ? &*builtinTypeName_ : nullptr;
}

} // namespace ast
