#include "ComparisonExpression.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "Operator.h"
#include "types/Type.h"

namespace ast {

ComparisonExpression::ComparisonExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> comparisonOperator,
        std::unique_ptr<Expression> rightHandSide) :
        DoubleOperandExpression(std::move(leftHandSide), std::move(rightHandSide), std::move(comparisonOperator))
{
    setType(type::signedInteger());
}

void ComparisonExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

symbols::LabelEntry* ComparisonExpression::getFalsyLabel(symbols::AnnotationStore& store) const {
    return store.label(this, symbols::LabelSlot::Falsy);
}

void ComparisonExpression::setFalsyLabel(symbols::AnnotationStore& store, symbols::LabelEntry falsyLabel) {
    store.setLabel(this, symbols::LabelSlot::Falsy, std::move(falsyLabel));
}

symbols::LabelEntry* ComparisonExpression::getTruthyLabel(symbols::AnnotationStore& store) const {
    return store.label(this, symbols::LabelSlot::Truthy);
}

void ComparisonExpression::setTruthyLabel(symbols::AnnotationStore& store, symbols::LabelEntry truthyLabel) {
    store.setLabel(this, symbols::LabelSlot::Truthy, std::move(truthyLabel));
}

} // namespace ast

