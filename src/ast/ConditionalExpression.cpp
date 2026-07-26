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

void ConditionalExpression::setFalsyLabel(semantic_analyzer::LabelEntry label) {
    falsyLabel = std::make_unique<semantic_analyzer::LabelEntry>(label);
}

semantic_analyzer::LabelEntry* ConditionalExpression::getFalsyLabel() const {
    return falsyLabel.get();
}

void ConditionalExpression::setExitLabel(semantic_analyzer::LabelEntry label) {
    exitLabel = std::make_unique<semantic_analyzer::LabelEntry>(label);
}

semantic_analyzer::LabelEntry* ConditionalExpression::getExitLabel() const {
    return exitLabel.get();
}

bool ConditionalExpression::evaluateConstant(long& value) const {
    long condValue;
    if (!condition->evaluateConstant(condValue)) {
        return false;
    }
    return condValue ? trueExpression->evaluateConstant(value) : falseExpression->evaluateConstant(value);
}

} // namespace ast
