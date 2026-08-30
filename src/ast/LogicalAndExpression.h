#ifndef _LOG_AND_EXPR_NODE_H_
#define _LOG_AND_EXPR_NODE_H_

#include <memory>

#include "LogicalExpression.h"

namespace ast {

class LogicalAndExpression: public LogicalExpression {
public:
    LogicalAndExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Expression> rightHandSide);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    bool evaluateConstant(type::IntegerConstant& value) const override;
};

} // namespace ast

#endif // _LOG_AND_EXPR_NODE_H_
