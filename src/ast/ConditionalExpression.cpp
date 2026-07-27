#include "ConditionalExpression.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

ConditionalExpression::ConditionalExpression(std::unique_ptr<Expression> condition,
        std::unique_ptr<Expression> trueExpression,
        std::unique_ptr<Expression> falseExpression) :
        condition { std::move(condition) },
        trueExpression { std::move(trueExpression) },
        falseExpression { std::move(falseExpression) } {
}

void ConditionalExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void ConditionalExpression::visitCondition(AbstractSyntaxTreeVisitor& visitor) {
    condition->accept(visitor);
}

void ConditionalExpression::visitTrueExpression(AbstractSyntaxTreeVisitor& visitor) {
    trueExpression->accept(visitor);
}

void ConditionalExpression::visitFalseExpression(AbstractSyntaxTreeVisitor& visitor) {
    falseExpression->accept(visitor);
}

symbols::ValueEntry* ConditionalExpression::conditionSymbol(symbols::AnnotationStore& store) const {
    return condition->getResultSymbol(store);
}

symbols::ValueEntry* ConditionalExpression::trueSymbol(symbols::AnnotationStore& store) const {
    return trueExpression->getResultSymbol(store);
}

symbols::ValueEntry* ConditionalExpression::falseSymbol(symbols::AnnotationStore& store) const {
    return falseExpression->getResultSymbol(store);
}

translation_unit::Context ConditionalExpression::getContext() const {
    return condition->getContext();
}

void ConditionalExpression::setFalsyLabel(symbols::AnnotationStore& store, symbols::LabelEntry label) {
    store.setLabel(this, symbols::LabelSlot::Falsy, std::move(label));
}

symbols::LabelEntry* ConditionalExpression::getFalsyLabel(symbols::AnnotationStore& store) const {
    return store.label(this, symbols::LabelSlot::Falsy);
}

void ConditionalExpression::setExitLabel(symbols::AnnotationStore& store, symbols::LabelEntry label) {
    store.setLabel(this, symbols::LabelSlot::Exit, std::move(label));
}

symbols::LabelEntry* ConditionalExpression::getExitLabel(symbols::AnnotationStore& store) const {
    return store.label(this, symbols::LabelSlot::Exit);
}

bool ConditionalExpression::evaluateConstant(long& value) const {
    long condValue;
    if (!condition->evaluateConstant(condValue)) {
        return false;
    }
    return condValue ? trueExpression->evaluateConstant(value) : falseExpression->evaluateConstant(value);
}

} // namespace ast
