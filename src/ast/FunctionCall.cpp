#include "FunctionCall.h"

#include <algorithm>

#include "../code_generator/FunctionEntry.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "AssignmentExpressionList.h"
#include "Operator.h"

namespace ast {

FunctionCall::FunctionCall(std::unique_ptr<Expression> postfixExpression,
        std::unique_ptr<AssignmentExpressionList> argumentList) :
        SingleOperandExpression { std::move(postfixExpression), std::unique_ptr<Operator> { new Operator("()") } },
        argumentList { std::move(argumentList) }
{
}

FunctionCall::~FunctionCall() {
}

void FunctionCall::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

AssignmentExpressionList* FunctionCall::getArgumentList() const {
    return argumentList.get();
}

void FunctionCall::setSymbol(code_generator::FunctionEntry symbol) {
    this->symbol = std::make_unique<code_generator::FunctionEntry>(symbol);
}

code_generator::FunctionEntry* FunctionCall::getSymbol() const {
    return symbol.get();
}

} /* namespace ast */
