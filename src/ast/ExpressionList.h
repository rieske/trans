#ifndef EXPRESSIONLIST_H_
#define EXPRESSIONLIST_H_

#include <memory>

#include "DoubleOperandExpression.h"

namespace ast {

class ExpressionList: public DoubleOperandExpression {
public:
    ExpressionList(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Expression> rightHandSide);
    virtual ~ExpressionList();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    ExprKind exprKind() const override { return ExprKind::Comma; }
    // C: the comma operator's result is never an lvalue, whatever its operands are.
    bool isLval() const override { return false; }
    std::optional<type::Type> typeAtParseTime(const ParseEnvironment& environment) const override;
};

} // namespace ast

#endif // EXPRESSIONLIST_H_
