#include "LogicalExpression.h"

#include <algorithm>

#include "BasicType.h"

namespace semantic_analyzer {

LogicalExpression::LogicalExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Expression> rightHandSide) :
        leftHandSide { std::move(leftHandSide) },
        rightHandSide { std::move(rightHandSide) } {
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

} /* namespace semantic_analyzer */
