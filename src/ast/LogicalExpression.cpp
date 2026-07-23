#include "LogicalExpression.h"
#include "types/Type.h"
#include "symbols/AnnotationStore.h"
namespace ast {
LogicalExpression::LogicalExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> logicalOperator,
        std::unique_ptr<Expression> rightHandSide)
    : DoubleOperandExpression{std::move(leftHandSide), std::move(rightHandSide), std::move(logicalOperator)} {
    setType(type::signedInteger());
}
LogicalExpression::~LogicalExpression() = default;
void LogicalExpression::setExitLabel(symbols::LabelEntry exitLabel) {
    symbols::AnnotationStore::current().setLabel(this, symbols::LabelSlot::Exit, std::move(exitLabel));
}
symbols::LabelEntry* LogicalExpression::getExitLabel() const {
    return symbols::AnnotationStore::current().label(this, symbols::LabelSlot::Exit);
}
} // namespace ast
