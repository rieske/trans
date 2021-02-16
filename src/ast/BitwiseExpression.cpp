#include "BitwiseExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "Operator.h"

namespace ast {

const std::string BitwiseExpression::AND { "<and_exp>" };
const std::string BitwiseExpression::OR { "<inclusive_or_exp>" };
const std::string BitwiseExpression::XOR { "<exclusive_or_exp>" };

BitwiseExpression::BitwiseExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> bitwiseOperator,
        std::unique_ptr<Expression> rightHandSide) :
        DoubleOperandExpression(std::move(leftHandSide), std::move(rightHandSide), std::move(bitwiseOperator)) {
}

void BitwiseExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace ast
