#include "ComparisonExpression.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "BasicType.h"
#include "Operator.h"

namespace ast {

const std::string ComparisonExpression::DIFFERENCE { "<ml_expr>" };
const std::string ComparisonExpression::EQUALITY { "<eq_expr>" };

ComparisonExpression::ComparisonExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> comparisonOperator,
        std::unique_ptr<Expression> rightHandSide) :
        DoubleOperandExpression(std::move(leftHandSide), std::move(rightHandSide), std::move(comparisonOperator)) {
    setTypeInfo( { BasicType::INTEGER });
}

void ComparisonExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

code_generator::LabelEntry* ComparisonExpression::getFalsyLabel() const {
    return falsyLabel;
}

void ComparisonExpression::setFalsyLabel(code_generator::LabelEntry* falsyLabel) {
    this->falsyLabel = falsyLabel;
}

code_generator::LabelEntry* ComparisonExpression::getTruthyLabel() const {
    return truthyLabel;
}

void ComparisonExpression::setTruthyLabel(code_generator::LabelEntry* truthyLabel) {
    this->truthyLabel = truthyLabel;
}

}

