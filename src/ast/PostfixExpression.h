#ifndef _POSTFIX_EXPR_NODE_H_
#define _POSTFIX_EXPR_NODE_H_

#include <memory>

#include "UnaryOpExpression.h"
#include "types/Operator.h"

namespace ast {

class PostfixExpression: public UnaryOpExpression<type::IncDec> {
public:
    PostfixExpression(std::unique_ptr<Expression> postfixExpression, type::IncDec op);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    ExprKind exprKind() const override { return ExprKind::Postfix; }
};

} // namespace ast

#endif // _POSTFIX_EXPR_NODE_H_
