#include "ComparisonExpression.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "Operator.h"
#include "../types/IntegralType.h"

namespace ast {

const std::string ComparisonExpression::RELATIONAL { "<relational_exp>" };
const std::string ComparisonExpression::EQUALITY { "<equality_exp>" };

ComparisonExpression::ComparisonExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> comparisonOperator,
        std::unique_ptr<Expression> rightHandSide) :
        DoubleOperandExpression(std::move(leftHandSide), std::move(rightHandSide), std::move(comparisonOperator))
{
    auto integer = IntegralType::newSignedInteger();
    setType(*integer);
}

void ComparisonExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

semantic_analyzer::LabelEntry* ComparisonExpression::getFalsyLabel() const {
    return falsyLabel.get();
}

void ComparisonExpression::setFalsyLabel(semantic_analyzer::LabelEntry falsyLabel) {
    this->falsyLabel = std::unique_ptr<semantic_analyzer::LabelEntry> { new semantic_analyzer::LabelEntry { falsyLabel } };
}

semantic_analyzer::LabelEntry* ComparisonExpression::getTruthyLabel() const {
    return truthyLabel.get();
}

void ComparisonExpression::setTruthyLabel(semantic_analyzer::LabelEntry truthyLabel) {
    this->truthyLabel = std::unique_ptr<semantic_analyzer::LabelEntry> { new semantic_analyzer::LabelEntry { truthyLabel } };
}

}

