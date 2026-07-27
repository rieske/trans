#include "LogicalExpression.h"

namespace ast {

LogicalExpression::LogicalExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> logicalOperator,
        std::unique_ptr<Expression> rightHandSide) :
        DoubleOperandExpression { std::move(leftHandSide), std::move(rightHandSide), std::move(logicalOperator) }
{
    setType(type::signedInteger());
}

LogicalExpression::~LogicalExpression() {
}

void LogicalExpression::setExitLabel(symbols::AnnotationStore& store, symbols::LabelEntry exitLabel) {
    store.setLabel(this, symbols::LabelSlot::Exit, std::move(exitLabel));
}

symbols::LabelEntry* LogicalExpression::getExitLabel(symbols::AnnotationStore& store) const {
    return store.label(this, symbols::LabelSlot::Exit);
}

} // namespace ast

