#include "ComparisonExpression.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "BasicType.h"

namespace semantic_analyzer {

const std::string ComparisonExpression::DIFFERENCE { "<ml_expr>" };
const std::string ComparisonExpression::EQUALITY { "<eq_expr>" };

ComparisonExpression::ComparisonExpression(std::unique_ptr<Expression> leftHandSide, TerminalSymbol comparisonOperator,
        std::unique_ptr<Expression> rightHandSide) :
        leftHandSide { std::move(leftHandSide) },
        comparisonOperator { comparisonOperator },
        rightHandSide { std::move(rightHandSide) } {
    setTypeInfo( { BasicType::INTEGER });
}

void ComparisonExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

SymbolEntry* ComparisonExpression::getFalsyLabel() const {
    return falsyLabel;
}

void ComparisonExpression::setFalsyLabel(SymbolEntry* falsyLabel) {
    this->falsyLabel = falsyLabel;
}

SymbolEntry* ComparisonExpression::getTruthyLabel() const {
    return truthyLabel;
}

void ComparisonExpression::setTruthyLabel(SymbolEntry* truthyLabel) {
    this->truthyLabel = truthyLabel;
}

}

