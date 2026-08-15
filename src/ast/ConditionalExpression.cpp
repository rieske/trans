#include "ConditionalExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "ParseEnvironment.h"
#include "types/TypeQuery.h"

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

std::optional<type::Type> ConditionalExpression::typeAtParseTime(const ParseEnvironment& environment) const {
    if (!condition->typeAtParseTime(environment)) {
        return std::nullopt;
    }
    auto trueType = trueExpression->typeAtParseTime(environment);
    auto falseType = falseExpression->typeAtParseTime(environment);
    if (!trueType || !falseType) {
        return std::nullopt;
    }
    return type::conditionalResultType(*trueType, *falseType);
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

bool ConditionalExpression::evaluateConstant(type::IntegerConstant& value) const {
    type::IntegerConstant condValue;
    if (!condition->evaluateConstant(condValue)) {
        return false;
    }
    Expression* arm = type::isZero(condValue) ? falseExpression.get() : trueExpression.get();
    if (!arm->evaluateConstant(value)) {
        return false;
    }
    if (hasExpressionType()) {
        value = type::convert(value, getType());
    }
    return true;
}

} // namespace ast
