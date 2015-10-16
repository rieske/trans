#include "ast/PostfixExpression.h"

#include <algorithm>

#include "ast/AbstractSyntaxTreeVisitor.h"
#include "ast/Operator.h"

namespace ast {

const std::string PostfixExpression::ID { "<postfix_exp>" };

PostfixExpression::PostfixExpression(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<Operator> postfixOperator) :
        SingleOperandExpression(std::move(postfixExpression), std::move(postfixOperator)) {
}

void PostfixExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

}
