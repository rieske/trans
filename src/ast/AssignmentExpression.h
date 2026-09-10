#ifndef _A_EXPR_NODE_H_
#define _A_EXPR_NODE_H_

#include <memory>

#include "BinaryOpExpression.h"
#include "types/Operator.h"

namespace ast {

class AssignmentExpression: public BinaryOpExpression<type::AssignOp> {
public:
    AssignmentExpression(std::unique_ptr<Expression> leftHandSide, type::AssignOp op,
            std::unique_ptr<Expression> rightHandSide);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    ExprKind exprKind() const override { return ExprKind::Assignment; }
    std::optional<type::Type> typeAtParseTime(const ParseEnvironment& environment) const override;

    symbols::ValueEntry* leftOperandLvalueSymbol(symbols::AnnotationStore& store) const;
};

} // namespace ast

#endif // _A_EXPR_NODE_H_
