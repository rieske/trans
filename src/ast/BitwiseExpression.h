#ifndef _BITWISE_EXPRESSION_NODE_H_
#define _BITWISE_EXPRESSION_NODE_H_

#include <memory>

#include "BinaryOpExpression.h"
#include "types/Operator.h"

namespace ast {

class BitwiseExpression: public BinaryOpExpression<type::BitwiseOp> {
public:
    BitwiseExpression(std::unique_ptr<Expression> leftHandSide, type::BitwiseOp op,
            std::unique_ptr<Expression> rightHandSide);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    ExprKind exprKind() const override { return ExprKind::Bitwise; }
    std::optional<type::Type> typeAtParseTime(const ParseEnvironment& environment) const override;
    bool evaluateConstant(type::IntegerConstant& value) const override {
        return foldOperands(value, type::asBinary(op()));
    }
};

} // namespace ast

#endif // _BITWISE_EXPRESSION_NODE_H_
