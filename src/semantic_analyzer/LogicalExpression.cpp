#include "LogicalExpression.h"

#include <algorithm>

namespace semantic_analyzer {

LogicalExpression::LogicalExpression(std::unique_ptr<Expression> leftHandSide,
        std::unique_ptr<Expression> rightHandSide, SymbolTable *st, unsigned ln) :
        Expression(st, ln),
        leftHandSide { std::move(leftHandSide) },
        rightHandSide { std::move(rightHandSide) } {
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
