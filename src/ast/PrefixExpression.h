#ifndef PREFIXEXPRESSION_H_
#define PREFIXEXPRESSION_H_

#include <memory>

#include "UnaryOpExpression.h"
#include "types/Operator.h"

namespace ast {

class PrefixExpression: public UnaryOpExpression<type::IncDec> {
public:
    PrefixExpression(type::IncDec op, std::unique_ptr<Expression> unaryExpression);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    ExprKind exprKind() const override { return ExprKind::Prefix; }
};

} // namespace ast

#endif // PREFIXEXPRESSION_H_
