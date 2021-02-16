#ifndef PREFIXEXPRESSION_H_
#define PREFIXEXPRESSION_H_

#include <memory>

#include "SingleOperandExpression.h"

namespace ast {

class PrefixExpression: public SingleOperandExpression {
public:
    PrefixExpression(std::unique_ptr<Operator> incrementOperator, std::unique_ptr<Expression> unaryExpression);
    virtual ~PrefixExpression();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
};

} // namespace ast

#endif // PREFIXEXPRESSION_H_
