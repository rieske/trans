#include "FunctionCall.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "AssignmentExpressionList.h"
#include "Operator.h"

namespace ast {

FunctionCall::FunctionCall(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<AssignmentExpressionList> argumentList) :
        SingleOperandExpression { std::move(postfixExpression), std::unique_ptr<Operator> { new Operator("()") } }, argumentList { std::move(argumentList) } {
}

FunctionCall::~FunctionCall() {
}

void FunctionCall::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

AssignmentExpressionList* FunctionCall::getArgumentList() const {
    return argumentList.get();
}

} /* namespace ast */
