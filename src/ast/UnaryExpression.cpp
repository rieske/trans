#include "UnaryExpression.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "Operator.h"

namespace ast {

const std::string UnaryExpression::ID { "<u_expr>" };

UnaryExpression::UnaryExpression(std::unique_ptr<Operator> unaryOperator, std::unique_ptr<Expression> castExpression) :
        SingleOperandExpression(std::move(castExpression), std::move(unaryOperator)) {
    lval = _operand->isLval();
}

void UnaryExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void UnaryExpression::setTruthyLabel(code_generator::LabelEntry* truthyLabel) {
    this->truthyLabel = truthyLabel;
}

code_generator::LabelEntry* UnaryExpression::getTruthyLabel() const {
    return truthyLabel;
}

void UnaryExpression::setFalsyLabel(code_generator::LabelEntry* falsyLabel) {
    this->falsyLabel = falsyLabel;
}

code_generator::LabelEntry* UnaryExpression::getFalsyLabel() const {
    return falsyLabel;
}

}
