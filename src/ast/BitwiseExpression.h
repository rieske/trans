#ifndef _BITWISE_EXPRESSION_NODE_H_
#define _BITWISE_EXPRESSION_NODE_H_

#include <memory>
#include <string>

#include "DoubleOperandExpression.h"

namespace ast {

class BitwiseExpression: public DoubleOperandExpression {
public:
    BitwiseExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> bitwiseOperator, std::unique_ptr<Expression> rightHandSide);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    static const std::string AND;
    static const std::string OR;
    static const std::string XOR;
};

} // namespace ast

#endif // _BITWISE_EXPRESSION_NODE_H_
