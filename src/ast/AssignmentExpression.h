#ifndef _A_EXPR_NODE_H_
#define _A_EXPR_NODE_H_

#include <memory>

#include "DoubleOperandExpression.h"

namespace ast {

class AssignmentExpression: public DoubleOperandExpression {
public:
    AssignmentExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> assignmentOperator, std::unique_ptr<Expression> rightHandSide);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    // C: assignment expression is never an lvalue. SA checks leftOperand->isLval().
    bool isLval() const override;

    // Pointer compound-assign scaling lives in symbols::PointerArithPlan on the store.
};

} // namespace ast

#endif // _A_EXPR_NODE_H_
