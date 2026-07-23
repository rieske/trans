#include "ConditionalExpression.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "symbols/AnnotationStore.h"
namespace ast {
ConditionalExpression::ConditionalExpression(std::unique_ptr<Expression> condition,
        std::unique_ptr<Expression> trueExpression, std::unique_ptr<Expression> falseExpression)
    : condition{std::move(condition)}, trueExpression{std::move(trueExpression)}, falseExpression{std::move(falseExpression)} {}
void ConditionalExpression::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
void ConditionalExpression::visitCondition(AbstractSyntaxTreeVisitor& visitor) { condition->accept(visitor); }
void ConditionalExpression::visitTrueExpression(AbstractSyntaxTreeVisitor& visitor) { trueExpression->accept(visitor); }
void ConditionalExpression::visitFalseExpression(AbstractSyntaxTreeVisitor& visitor) { falseExpression->accept(visitor); }
type::Type ConditionalExpression::conditionType() const { return condition->expressionType(); }
type::Type ConditionalExpression::trueType() const { return trueExpression->expressionType(); }
type::Type ConditionalExpression::falseType() const { return falseExpression->expressionType(); }
symbols::ValueEntry* ConditionalExpression::conditionSymbol() const { return condition->getResultSymbol(); }
symbols::ValueEntry* ConditionalExpression::trueSymbol() const { return trueExpression->getResultSymbol(); }
symbols::ValueEntry* ConditionalExpression::falseSymbol() const { return falseExpression->getResultSymbol(); }
translation_unit::Context ConditionalExpression::getContext() const { return condition->getContext(); }
void ConditionalExpression::setTruthyLabel(symbols::LabelEntry label) {
    symbols::AnnotationStore::current().setLabel(this, symbols::LabelSlot::Truthy, std::move(label));
}
symbols::LabelEntry* ConditionalExpression::getTruthyLabel() const {
    return symbols::AnnotationStore::current().label(this, symbols::LabelSlot::Truthy);
}
void ConditionalExpression::setFalsyLabel(symbols::LabelEntry label) {
    symbols::AnnotationStore::current().setLabel(this, symbols::LabelSlot::Falsy, std::move(label));
}
symbols::LabelEntry* ConditionalExpression::getFalsyLabel() const {
    return symbols::AnnotationStore::current().label(this, symbols::LabelSlot::Falsy);
}
void ConditionalExpression::setExitLabel(symbols::LabelEntry label) {
    symbols::AnnotationStore::current().setLabel(this, symbols::LabelSlot::Exit, std::move(label));
}
symbols::LabelEntry* ConditionalExpression::getExitLabel() const {
    return symbols::AnnotationStore::current().label(this, symbols::LabelSlot::Exit);
}
bool ConditionalExpression::evaluateConstant(long& value) const {
    long cond = 0;
    if (!condition->evaluateConstant(cond)) return false;
    return cond ? trueExpression->evaluateConstant(value) : falseExpression->evaluateConstant(value);
}
} // namespace ast
