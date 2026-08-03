#ifndef LOGICALEXPRESSION_H_
#define LOGICALEXPRESSION_H_

#include <memory>

#include "ast/DoubleOperandExpression.h"

namespace ast {

class LogicalExpression: public DoubleOperandExpression {
public:
    virtual ~LogicalExpression();

    std::optional<type::Type> typeAtParseTime(const ParseEnvironment& environment) const override;

protected:
    LogicalExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> logicalOperator, std::unique_ptr<Expression> rightHandSide);
};

} // namespace ast

#endif // LOGICALEXPRESSION_H_
