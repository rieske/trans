#include "ConditionalExpression.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

ConditionalExpression::ConditionalExpression(
        std::unique_ptr<Expression> condition,
        std::unique_ptr<Expression> trueExpression,
        std::unique_ptr<Expression> falseExpression) :
        condition { std::move(condition) },
        trueExpression { std::move(trueExpression) },
        falseExpression { std::move(falseExpression) }
{
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

type::Type ConditionalExpression::conditionType() const {
    return condition->getType();
}

type::Type ConditionalExpression::trueType() const {
    return trueExpression->getType();
}

type::Type ConditionalExpression::falseType() const {
    return falseExpression->getType();
}

semantic_analyzer::ValueEntry* ConditionalExpression::conditionSymbol() const {
    return condition->getResultSymbol();
}

semantic_analyzer::ValueEntry* ConditionalExpression::trueSymbol() const {
    return trueExpression->getResultSymbol();
}

semantic_analyzer::ValueEntry* ConditionalExpression::falseSymbol() const {
    return falseExpression->getResultSymbol();
}

translation_unit::Context ConditionalExpression::getContext() const {
    return condition->getContext();
}

void ConditionalExpression::setTruthyLabel(semantic_analyzer::LabelEntry label) {
    truthyLabel = std::make_unique<semantic_analyzer::LabelEntry>(label);
}

semantic_analyzer::LabelEntry* ConditionalExpression::getTruthyLabel() const {
    return truthyLabel.get();
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
    long cond = 0;
    if (!condition->evaluateConstant(cond)) {
        return false;
    }
    if (cond) {
        return trueExpression->evaluateConstant(value);
    }
    return falseExpression->evaluateConstant(value);
}

} // namespace ast
