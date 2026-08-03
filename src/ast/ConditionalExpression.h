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
    std::optional<type::Type> typeAtParseTime(const ParseEnvironment& environment) const override;

    void visitCondition(AbstractSyntaxTreeVisitor& visitor);
    void visitTrueExpression(AbstractSyntaxTreeVisitor& visitor);
    void visitFalseExpression(AbstractSyntaxTreeVisitor& visitor);

    Expression* getCondition() const { return condition.get(); }
    Expression* getTrueExpression() const { return trueExpression.get(); }
    Expression* getFalseExpression() const { return falseExpression.get(); }

    translation_unit::Context getContext() const override;

    void setFalsyLabel(symbols::AnnotationStore& store, symbols::LabelEntry label);
    symbols::LabelEntry* getFalsyLabel(symbols::AnnotationStore& store) const;
    void setExitLabel(symbols::AnnotationStore& store, symbols::LabelEntry label);
    symbols::LabelEntry* getExitLabel(symbols::AnnotationStore& store) const;

    bool evaluateConstant(type::IntegerConstant& value) const override;

private:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Expression> trueExpression;
    std::unique_ptr<Expression> falseExpression;

};

} // namespace ast

#endif // CONDITIONALEXPRESSION_H_
