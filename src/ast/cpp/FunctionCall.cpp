#include "ast/FunctionCall.h"

#include <algorithm>

#include "ast/AbstractSyntaxTreeVisitor.h"
#include "ast/Operator.h"

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

} /* namespace ast */
