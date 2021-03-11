#ifndef _ADD_EXPR_NODE_H_
#define _ADD_EXPR_NODE_H_

#include <memory>

#include "DoubleOperandExpression.h"

namespace ast {

class ArithmeticExpression: public DoubleOperandExpression {
public:
    ArithmeticExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> arithmeticOperator, std::unique_ptr<Expression> rightHandSide);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
};

} // namespace ast

#endif // _ADD_EXPR_NODE_H_
