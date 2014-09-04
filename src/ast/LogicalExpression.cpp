#include "LogicalExpression.h"

#include <algorithm>

#include "BasicType.h"
#include "Operator.h"

namespace ast {

LogicalExpression::LogicalExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> logicalOperator, std::unique_ptr<Expression> rightHandSide) :
        DoubleOperandExpression { std::move(leftHandSide), std::move(rightHandSide), std::move(logicalOperator) } {
    setTypeInfo( { BasicType::INTEGER });
}

LogicalExpression::~LogicalExpression() {
}

void LogicalExpression::setExitLabel(SymbolEntry* exitLabel) {
    this->exitLabel = exitLabel;
}

SymbolEntry* LogicalExpression::getExitLabel() const {
    return exitLabel;
}

} /* namespace ast */
