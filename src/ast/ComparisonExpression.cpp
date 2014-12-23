#include "ComparisonExpression.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "types/BaseType.h"
#include "Operator.h"

namespace ast {

const std::string ComparisonExpression::DIFFERENCE { "<ml_expr>" };
const std::string ComparisonExpression::EQUALITY { "<eq_expr>" };

ComparisonExpression::ComparisonExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> comparisonOperator,
        std::unique_ptr<Expression> rightHandSide) :
        DoubleOperandExpression(std::move(leftHandSide), std::move(rightHandSide), std::move(comparisonOperator))
{
    setType( { BaseType::newInteger() });
}

void ComparisonExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

code_generator::LabelEntry* ComparisonExpression::getFalsyLabel() const {
    return falsyLabel.get();
}

void ComparisonExpression::setFalsyLabel(code_generator::LabelEntry falsyLabel) {
    this->falsyLabel = std::unique_ptr<code_generator::LabelEntry> { new code_generator::LabelEntry { falsyLabel } };
}

code_generator::LabelEntry* ComparisonExpression::getTruthyLabel() const {
    return truthyLabel.get();
}

void ComparisonExpression::setTruthyLabel(code_generator::LabelEntry truthyLabel) {
    this->truthyLabel = std::unique_ptr<code_generator::LabelEntry> { new code_generator::LabelEntry { truthyLabel } };
}

}

