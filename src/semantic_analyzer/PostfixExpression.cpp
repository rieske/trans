#include "PostfixExpression.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

const std::string PostfixExpression::ID { "<postfix_expr>" };

PostfixExpression::PostfixExpression(std::unique_ptr<Expression> postfixExpression, TerminalSymbol postfixOperator) :
        postfixExpression { std::move(postfixExpression) },
        postfixOperator { postfixOperator } {
}

void PostfixExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

}
