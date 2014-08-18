#include "FunctionCall.h"

#include <algorithm>
#include <string>

#include "AbstractSyntaxTreeVisitor.h"

#include "AssignmentExpressionList.h"

namespace semantic_analyzer {

FunctionCall::FunctionCall(std::unique_ptr<Expression> postfixExpression,
        std::unique_ptr<AssignmentExpressionList> argumentList) :
        callExpression { std::move(postfixExpression) },
        argumentList { std::move(argumentList) } {
}

FunctionCall::~FunctionCall() {
}

void FunctionCall::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} /* namespace semantic_analyzer */
