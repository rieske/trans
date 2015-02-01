#include "UnaryExpression.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "Operator.h"

namespace ast {

const std::string UnaryExpression::ID { "<unary_exp>" };

UnaryExpression::UnaryExpression(std::unique_ptr<Operator> unaryOperator, std::unique_ptr<Expression> castExpression) :
        SingleOperandExpression(std::move(castExpression), std::move(unaryOperator))
{
    lval = _operand->isLval();
}

void UnaryExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void UnaryExpression::setTruthyLabel(code_generator::LabelEntry truthyLabel) {
    this->truthyLabel = std::unique_ptr<code_generator::LabelEntry> { new code_generator::LabelEntry { truthyLabel } };
}

code_generator::LabelEntry* UnaryExpression::getTruthyLabel() const {
    return truthyLabel.get();
}

void UnaryExpression::setFalsyLabel(code_generator::LabelEntry falsyLabel) {
    this->falsyLabel = std::unique_ptr<code_generator::LabelEntry> { new code_generator::LabelEntry { falsyLabel } };
}

code_generator::LabelEntry* UnaryExpression::getFalsyLabel() const {
    return falsyLabel.get();
}

}
