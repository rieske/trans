#ifndef _ADD_EXPR_NODE_H_
#define _ADD_EXPR_NODE_H_

#include <memory>
#include <string>

#include "DoubleOperandExpression.h"

namespace ast {

class ArithmeticExpression: public DoubleOperandExpression {
public:
    ArithmeticExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> arithmeticOperator, std::unique_ptr<Expression> rightHandSide);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    static const std::string ADDITION;
    static const std::string MULTIPLICATION;
};

} // namespace ast

#endif // _ADD_EXPR_NODE_H_
