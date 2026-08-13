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

    translation_unit::Context getContext() const override;

    Block& body() { return *body_; }
    const Block& body() const { return *body_; }

    // Last expression-statement; owned by body_.
    void setValueSource(Expression* source) { valueSource_ = source; }
    Expression* valueSource() const { return valueSource_; }

private:
    translation_unit::Context context_;
    std::unique_ptr<Block> body_;
    Expression* valueSource_ { nullptr };
};

} // namespace ast

#endif
