#ifndef CONDITIONALEXPRESSION_H_
#define CONDITIONALEXPRESSION_H_

#include <memory>

#include "Expression.h"
#include "semantic_analyzer/LabelEntry.h"

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

    semantic_analyzer::ValueEntry* conditionSymbol() const;
    semantic_analyzer::ValueEntry* trueSymbol() const;
    semantic_analyzer::ValueEntry* falseSymbol() const;

    translation_unit::Context getContext() const override;

    void setTruthyLabel(semantic_analyzer::LabelEntry label);
    semantic_analyzer::LabelEntry* getTruthyLabel() const;
    void setFalsyLabel(semantic_analyzer::LabelEntry label);
    semantic_analyzer::LabelEntry* getFalsyLabel() const;
    void setExitLabel(semantic_analyzer::LabelEntry label);
    semantic_analyzer::LabelEntry* getExitLabel() const;

    bool evaluateConstant(long& value) const override;

private:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Expression> trueExpression;
    std::unique_ptr<Expression> falseExpression;

    std::unique_ptr<semantic_analyzer::LabelEntry> truthyLabel { nullptr };
    std::unique_ptr<semantic_analyzer::LabelEntry> falsyLabel { nullptr };
    std::unique_ptr<semantic_analyzer::LabelEntry> exitLabel { nullptr };
};

} // namespace ast

#endif // CONDITIONALEXPRESSION_H_
