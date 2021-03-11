#ifndef _S_EXPR_NODE_H_
#define _S_EXPR_NODE_H_

#include <memory>

#include "ast/DoubleOperandExpression.h"

namespace ast {

class ShiftExpression: public DoubleOperandExpression {
public:
    ShiftExpression(std::unique_ptr<Expression> shiftExpression, std::unique_ptr<Operator> shiftOperator, std::unique_ptr<Expression> additionExpression);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
};

} // namespace ast

#endif // _S_EXPR_NODE_H_
