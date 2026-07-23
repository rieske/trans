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

void FunctionCall::setSymbol(semantic_analyzer::FunctionEntry symbol) {
    this->symbol = std::make_unique<semantic_analyzer::FunctionEntry>(symbol);
}

semantic_analyzer::FunctionEntry* FunctionCall::getSymbol() const {
    return symbol.get();
}

void FunctionCall::setBuiltinKind(BuiltinKind kind) {
    builtinKind = kind;
}

FunctionCall::BuiltinKind FunctionCall::getBuiltinKind() const {
    return builtinKind;
}

void FunctionCall::setVaArgResultType(type::Type type) {
    vaArgResultType = std::move(type);
}

const type::Type* FunctionCall::getVaArgResultType() const {
    if (!vaArgResultType) {
        return nullptr;
    }
    return &*vaArgResultType;
}

void FunctionCall::setIndirect(bool indirect) {
    indirectCall = indirect;
}

bool FunctionCall::isIndirect() const {
    return indirectCall;
}

} // namespace ast
