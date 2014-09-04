#include "ArithmeticExpression.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "Operator.h"

namespace ast {

const std::string ArithmeticExpression::ADDITION { "<add_expr>" };
const std::string ArithmeticExpression::MULTIPLICATION { "<factor>" };

ArithmeticExpression::ArithmeticExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> arithmeticOperator,
        std::unique_ptr<Expression> rightHandSide) :
        DoubleOperandExpression(std::move(leftHandSide), std::move(rightHandSide), std::move(arithmeticOperator)) {
}

void ArithmeticExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

}
