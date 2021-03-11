#ifndef _A_EXPR_NODE_H_
#define _A_EXPR_NODE_H_

#include <memory>

#include "DoubleOperandExpression.h"

namespace ast {

class AssignmentExpression: public DoubleOperandExpression {
public:
    AssignmentExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> assignmentOperator, std::unique_ptr<Expression> rightHandSide);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    semantic_analyzer::ValueEntry* leftOperandLvalueSymbol() const;
};

} // namespace ast

#endif // _A_EXPR_NODE_H_
