#include "LogicalExpression.h"

#include <algorithm>

#include "types/BaseType.h"
#include "Operator.h"

namespace ast {

LogicalExpression::LogicalExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> logicalOperator, std::unique_ptr<Expression> rightHandSide) :
        DoubleOperandExpression { std::move(leftHandSide), std::move(rightHandSide), std::move(logicalOperator) } {
    setType( { BaseType::newInteger() });
}

LogicalExpression::~LogicalExpression() {
}

void LogicalExpression::setExitLabel(code_generator::LabelEntry* exitLabel) {
    this->exitLabel = exitLabel;
}

code_generator::LabelEntry* LogicalExpression::getExitLabel() const {
    return exitLabel;
}

} /* namespace ast */
