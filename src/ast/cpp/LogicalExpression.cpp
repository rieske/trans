#include "ast/LogicalExpression.h"

#include <algorithm>

#include "types/IntegralType.h"
#include "ast/Operator.h"

namespace ast {

LogicalExpression::LogicalExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> logicalOperator,
        std::unique_ptr<Expression> rightHandSide) :
        DoubleOperandExpression { std::move(leftHandSide), std::move(rightHandSide), std::move(logicalOperator) }
{
    auto integer = IntegralType::newSignedInteger();
    setType(*integer);
}

LogicalExpression::~LogicalExpression() {
}

void LogicalExpression::setExitLabel(semantic_analyzer::LabelEntry exitLabel) {
    this->exitLabel = std::unique_ptr<semantic_analyzer::LabelEntry> { new semantic_analyzer::LabelEntry { exitLabel } };
}

semantic_analyzer::LabelEntry* LogicalExpression::getExitLabel() const {
    return exitLabel.get();
}

} /* namespace ast */
