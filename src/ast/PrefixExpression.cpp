#include "PrefixExpression.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "Operator.h"

namespace ast {

PrefixExpression::PrefixExpression(std::unique_ptr<Operator> incrementOperator, std::unique_ptr<Expression> unaryExpression) :
        SingleOperandExpression(std::move(unaryExpression), std::move(incrementOperator)) {
}

PrefixExpression::~PrefixExpression() {
}

void PrefixExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} /* namespace ast */
