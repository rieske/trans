#ifndef _ADD_EXPR_NODE_H_
#define _ADD_EXPR_NODE_H_

#include <memory>

#include "BinaryOpExpression.h"
#include "types/Operator.h"

namespace ast {

class ArithmeticExpression: public BinaryOpExpression<type::ArithmeticOp> {
public:
    ArithmeticExpression(std::unique_ptr<Expression> leftHandSide, type::ArithmeticOp op,
            std::unique_ptr<Expression> rightHandSide);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    ExprKind exprKind() const override { return ExprKind::Arithmetic; }
    std::optional<type::Type> typeAtParseTime(const ParseEnvironment& environment) const override;
    bool evaluateConstant(type::IntegerConstant& value) const override {
        return foldOperands(value, type::asBinary(op()));
    }
};

} // namespace ast

#endif // _ADD_EXPR_NODE_H_
