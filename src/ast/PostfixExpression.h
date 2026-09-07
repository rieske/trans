#ifndef _POSTFIX_EXPR_NODE_H_
#define _POSTFIX_EXPR_NODE_H_

#include <memory>

#include "UnaryOpExpression.h"
#include "symbols/AnnotationStore.h"
#include "types/Operator.h"

namespace ast {

class PostfixExpression: public UnaryOpExpression<type::IncDec> {
public:
    PostfixExpression(std::unique_ptr<Expression> postfixExpression, type::IncDec op);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    ExprKind exprKind() const override { return ExprKind::Postfix; }

    void setPreOperationSymbol(symbols::AnnotationStore& store, symbols::ValueEntry resultSymbol);
    symbols::ValueEntry* getPreOperationSymbol(symbols::AnnotationStore& store) const;
};

} // namespace ast

#endif // _POSTFIX_EXPR_NODE_H_
