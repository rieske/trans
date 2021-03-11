#ifndef _POSTFIX_EXPR_NODE_H_
#define _POSTFIX_EXPR_NODE_H_

#include <memory>

#include "SingleOperandExpression.h"

namespace ast {

class PostfixExpression: public SingleOperandExpression {
public:
    PostfixExpression(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<Operator> postfixOperator);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void setPreOperationSymbol(semantic_analyzer::ValueEntry resultSymbol);
    semantic_analyzer::ValueEntry* getPreOperationSymbol() const;

private:
    std::unique_ptr<semantic_analyzer::ValueEntry> preOperationSymbol { nullptr };
};

} // namespace ast

#endif // _POSTFIX_EXPR_NODE_H_
