#ifndef _LOG_AND_EXPR_NODE_H_
#define _LOG_AND_EXPR_NODE_H_

#include <memory>
#include <string>

#include "LogicalExpression.h"

namespace ast {

class LogicalAndExpression: public LogicalExpression {
public:
    LogicalAndExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Expression> rightHandSide);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
};

} // namespace ast

#endif // _LOG_AND_EXPR_NODE_H_
