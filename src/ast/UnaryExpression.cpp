#include "UnaryExpression.h"

#include "AbstractSyntaxTreeVisitor.h"

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

void UnaryExpression::setTruthyLabel(semantic_analyzer::LabelEntry truthyLabel) {
    this->truthyLabel = std::unique_ptr<semantic_analyzer::LabelEntry> {
            new semantic_analyzer::LabelEntry { truthyLabel } };
}

semantic_analyzer::LabelEntry* UnaryExpression::getTruthyLabel() const {
    return truthyLabel.get();
}

void UnaryExpression::setFalsyLabel(semantic_analyzer::LabelEntry falsyLabel) {
    this->falsyLabel =
            std::unique_ptr<semantic_analyzer::LabelEntry> { new semantic_analyzer::LabelEntry { falsyLabel } };
}

semantic_analyzer::LabelEntry* UnaryExpression::getFalsyLabel() const {
    return falsyLabel.get();
}

void UnaryExpression::setLvalueSymbol(semantic_analyzer::ValueEntry lvalueSymbol) {
    this->lvalueSymbol = std::make_unique<semantic_analyzer::ValueEntry>(lvalueSymbol);
}

semantic_analyzer::ValueEntry* UnaryExpression::getLvalueSymbol() const {
    return lvalueSymbol.get();
}

} // namespace ast

