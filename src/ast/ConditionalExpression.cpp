#include "ConditionalExpression.h"
#include "AbstractSyntaxTreeVisitor.h"
namespace ast {
ConditionalExpression::ConditionalExpression(std::unique_ptr<Expression> condition,
        std::unique_ptr<Expression> trueExpression, std::unique_ptr<Expression> falseExpression)
    : condition{std::move(condition)}, trueExpression{std::move(trueExpression)}, falseExpression{std::move(falseExpression)} {}
void ConditionalExpression::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
void ConditionalExpression::visitCondition(AbstractSyntaxTreeVisitor& visitor) { condition->accept(visitor); }
void ConditionalExpression::visitTrueExpression(AbstractSyntaxTreeVisitor& visitor) { trueExpression->accept(visitor); }
void ConditionalExpression::visitFalseExpression(AbstractSyntaxTreeVisitor& visitor) { falseExpression->accept(visitor); }
translation_unit::Context ConditionalExpression::getContext() const { return condition->getContext(); }
bool ConditionalExpression::evaluateConstant(long& value) const {
    long cond = 0;
    if (!condition->evaluateConstant(cond)) return false;
    return cond ? trueExpression->evaluateConstant(value) : falseExpression->evaluateConstant(value);
}
} // namespace ast
