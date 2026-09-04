#ifndef PREFIXEXPRESSION_H_
#define PREFIXEXPRESSION_H_

#include <memory>
#include <string>

#include "UnaryOpExpression.h"

namespace ast {

class PrefixExpression: public UnaryOpExpression {
public:
    PrefixExpression(std::string lexeme, std::unique_ptr<Expression> unaryExpression);
    virtual ~PrefixExpression();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    ExprKind exprKind() const override { return ExprKind::Prefix; }
};

} // namespace ast

#endif // PREFIXEXPRESSION_H_
