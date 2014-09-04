#ifndef _A_EXPR_NODE_H_
#define _A_EXPR_NODE_H_

#include <memory>
#include <string>

#include "DoubleOperandExpression.h"

namespace ast {

class AssignmentExpression: public DoubleOperandExpression {
public:
    AssignmentExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> assignmentOperator, std::unique_ptr<Expression> rightHandSide);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    static const std::string ID;
};

}

#endif // _A_EXPR_NODE_H_
