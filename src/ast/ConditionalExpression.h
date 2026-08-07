#ifndef CONDITIONALEXPRESSION_H_
#define CONDITIONALEXPRESSION_H_

#include <memory>

#include "Expression.h"

namespace ast {

class AbstractSyntaxTreeVisitor;

// cond ? thenExpr : elseExpr
class ConditionalExpression: public Expression {
public:
    ConditionalExpression(
            std::unique_ptr<Expression> condition,
            std::unique_ptr<Expression> trueExpression,
            std::unique_ptr<Expression> falseExpression);
    virtual ~ConditionalExpression() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void visitCondition(AbstractSyntaxTreeVisitor& visitor);
    void visitTrueExpression(AbstractSyntaxTreeVisitor& visitor);
    void visitFalseExpression(AbstractSyntaxTreeVisitor& visitor);

    Expression* getCondition() const { return condition.get(); }
    Expression* getTrueExpression() const { return trueExpression.get(); }
    Expression* getFalseExpression() const { return falseExpression.get(); }

    translation_unit::Context getContext() const override;

    bool evaluateConstant(long& value) const override;

private:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Expression> trueExpression;
    std::unique_ptr<Expression> falseExpression;

};

} // namespace ast

#endif // CONDITIONALEXPRESSION_H_
