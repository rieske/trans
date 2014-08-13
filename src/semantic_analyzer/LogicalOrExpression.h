#ifndef _LOG_EXPR_NODE_H_
#define _LOG_EXPR_NODE_H_

#include <memory>
#include <string>

#include "LogicalExpression.h"

namespace semantic_analyzer {

class LogicalOrExpression: public LogicalExpression {
public:
    LogicalOrExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Expression> rightHandSide);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    static const std::string ID;
};

}

#endif // _LOG_EXPR_NODE_H_
