#include "BitwiseExpression.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "Operator.h"

namespace ast {

const std::string BitwiseExpression::AND { "<and_expr>" };
const std::string BitwiseExpression::OR { "<or_expr>" };
const std::string BitwiseExpression::XOR { "<xor_expr>" };

BitwiseExpression::BitwiseExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> bitwiseOperator,
        std::unique_ptr<Expression> rightHandSide) :
        DoubleOperandExpression(std::move(leftHandSide), std::move(rightHandSide), std::move(bitwiseOperator)) {
}

void BitwiseExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

}
