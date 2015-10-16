#include "ast/ShiftExpression.h"

#include <algorithm>

#include "ast/AbstractSyntaxTreeVisitor.h"
#include "ast/Operator.h"

namespace ast {

const std::string ShiftExpression::ID { "<shift_expression>" };

ShiftExpression::ShiftExpression(std::unique_ptr<Expression> shiftExpression, std::unique_ptr<Operator> shiftOperator,
        std::unique_ptr<Expression> additionExpression) :
        DoubleOperandExpression { std::move(shiftExpression), std::move(additionExpression), std::move(shiftOperator) } {
}

void ShiftExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

}

