#ifndef _BITWISE_EXPRESSION_NODE_H_
#define _BITWISE_EXPRESSION_NODE_H_

#include <memory>

#include "DoubleOperandExpression.h"

namespace ast {

class BitwiseExpression: public DoubleOperandExpression {
public:
    BitwiseExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> bitwiseOperator, std::unique_ptr<Expression> rightHandSide);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
};

} // namespace ast

#endif // _BITWISE_EXPRESSION_NODE_H_
