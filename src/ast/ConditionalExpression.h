#ifndef CONDITIONALEXPRESSION_H_
#define CONDITIONALEXPRESSION_H_

#include <memory>

#include "Expression.h"
#include "symbols/LabelEntry.h"

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

    type::Type conditionType() const;
    type::Type trueType() const;
    type::Type falseType() const;

    symbols::ValueEntry* conditionSymbol() const;
    symbols::ValueEntry* trueSymbol() const;
    symbols::ValueEntry* falseSymbol() const;

    translation_unit::Context getContext() const override;

    void setTruthyLabel(symbols::LabelEntry label);
    symbols::LabelEntry* getTruthyLabel() const;
    void setFalsyLabel(symbols::LabelEntry label);
    symbols::LabelEntry* getFalsyLabel() const;
    void setExitLabel(symbols::LabelEntry label);
    symbols::LabelEntry* getExitLabel() const;

    bool evaluateConstant(long& value) const override;

private:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Expression> trueExpression;
    std::unique_ptr<Expression> falseExpression;

};

} // namespace ast

#endif // CONDITIONALEXPRESSION_H_
