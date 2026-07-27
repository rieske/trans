#ifndef _POSTFIX_EXPR_NODE_H_
#define _POSTFIX_EXPR_NODE_H_

#include <memory>

#include "SingleOperandExpression.h"
#include "symbols/AnnotationStore.h"

namespace ast {

class PostfixExpression: public SingleOperandExpression {
public:
    PostfixExpression(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<Operator> postfixOperator);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void setPreOperationSymbol(symbols::AnnotationStore& store, symbols::ValueEntry resultSymbol);
    symbols::ValueEntry* getPreOperationSymbol(symbols::AnnotationStore& store) const;

private:
};

} // namespace ast

#endif // _POSTFIX_EXPR_NODE_H_
