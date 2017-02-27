#ifndef _POSTFIX_EXPR_NODE_H_
#define _POSTFIX_EXPR_NODE_H_

#include <memory>
#include <string>

#include "SingleOperandExpression.h"

namespace ast {

class PostfixExpression: public SingleOperandExpression {
public:
    PostfixExpression(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<Operator> postfixOperator);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    static const std::string ID;
};

}

#endif // _POSTFIX_EXPR_NODE_H_
