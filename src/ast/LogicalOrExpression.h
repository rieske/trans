#ifndef _LOG_EXPR_NODE_H_
#define _LOG_EXPR_NODE_H_

#include <memory>

#include "ast/LogicalExpression.h"

namespace ast {

class LogicalOrExpression: public LogicalExpression {
public:
    LogicalOrExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Expression> rightHandSide);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
};

} // namespace ast

#endif // _LOG_EXPR_NODE_H_
