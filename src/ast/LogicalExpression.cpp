#include "LogicalExpression.h"

#include <algorithm>

#include "Operator.h"
#include "types/BaseType.h"
#include "types/Type.h"

namespace ast {

LogicalExpression::LogicalExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> logicalOperator,
        std::unique_ptr<Expression> rightHandSide) :
        DoubleOperandExpression { std::move(leftHandSide), std::move(rightHandSide), std::move(logicalOperator) }
{
    setType( { BaseType::newInteger() });
}

LogicalExpression::~LogicalExpression() {
}

void LogicalExpression::setExitLabel(code_generator::LabelEntry exitLabel) {
    this->exitLabel = std::unique_ptr<code_generator::LabelEntry> { new code_generator::LabelEntry { exitLabel } };
}

code_generator::LabelEntry* LogicalExpression::getExitLabel() const {
    return exitLabel.get();
}

} /* namespace ast */
