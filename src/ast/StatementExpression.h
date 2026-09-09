#ifndef AST_STATEMENTEXPRESSION_H_
#define AST_STATEMENTEXPRESSION_H_

#include <memory>

#include "Block.h"
#include "Expression.h"

namespace ast {

// GNU statement-expression: ({ ... }). Value is the last expression-statement.
class StatementExpression: public Expression {
public:
    StatementExpression(translation_unit::Context context, std::unique_ptr<Block> body);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    ExprKind exprKind() const override { return ExprKind::StatementExpression; }
    std::optional<type::Type> typeAtParseTime(const ParseEnvironment& environment) const override;

    // GNU: the value of a statement expression is not an lvalue.
    bool isLval() const override { return false; }

    translation_unit::Context getContext() const override;

    Block& body() { return *body_; }
    const Block& body() const { return *body_; }

    // GNU: the value is the last block item, when that item is an expression.
    Expression* valueExpression();
    const Expression* valueExpression() const;

private:
    translation_unit::Context context_;
    std::unique_ptr<Block> body_;
};

} // namespace ast

#endif
