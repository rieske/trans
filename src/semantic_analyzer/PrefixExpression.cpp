#include "PrefixExpression.h"

#include <algorithm>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

PrefixExpression::PrefixExpression(TerminalSymbol incrementOperator, std::unique_ptr<Expression> unaryExpression) :
        unaryExpression { std::move(unaryExpression) },
        incrementOperator { incrementOperator } {
    saveExpressionAttributes(*this->unaryExpression);
    value = "rval";
}

PrefixExpression::~PrefixExpression() {
}

void PrefixExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} /* namespace semantic_analyzer */
