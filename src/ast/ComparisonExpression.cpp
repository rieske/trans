#include "ComparisonExpression.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "Operator.h"
#include "types/Type.h"
#include "symbols/AnnotationStore.h"
namespace ast {
ComparisonExpression::ComparisonExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> comparisonOperator,
        std::unique_ptr<Expression> rightHandSide)
    : DoubleOperandExpression(std::move(leftHandSide), std::move(rightHandSide), std::move(comparisonOperator)) {
    setType(type::signedInteger());
}
void ComparisonExpression::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
void ComparisonExpression::setFalsyLabel(symbols::LabelEntry falsyLabel) {
    symbols::AnnotationStore::current().setLabel(this, symbols::LabelSlot::Falsy, std::move(falsyLabel));
}
symbols::LabelEntry* ComparisonExpression::getFalsyLabel() const {
    return symbols::AnnotationStore::current().label(this, symbols::LabelSlot::Falsy);
}
void ComparisonExpression::setTruthyLabel(symbols::LabelEntry truthyLabel) {
    symbols::AnnotationStore::current().setLabel(this, symbols::LabelSlot::Truthy, std::move(truthyLabel));
}
symbols::LabelEntry* ComparisonExpression::getTruthyLabel() const {
    return symbols::AnnotationStore::current().label(this, symbols::LabelSlot::Truthy);
}
} // namespace ast
