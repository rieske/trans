#include "ComparisonExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "ParseEnvironment.h"
#include "types/Type.h"

namespace ast {

ComparisonExpression::ComparisonExpression(std::unique_ptr<Expression> leftHandSide, std::string lexeme,
        std::unique_ptr<Expression> rightHandSide) :
        BinaryOpExpression(std::move(leftHandSide), std::move(lexeme), std::move(rightHandSide))
{
    setType(type::signedInteger());
}

void ComparisonExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

std::optional<type::Type> ComparisonExpression::typeAtParseTime(const ParseEnvironment& environment) const {
    return intIfOperandsType(environment);
}

symbols::LabelEntry* ComparisonExpression::getFalsyLabel(symbols::AnnotationStore& store) const {
    return store.label(this, symbols::LabelSlot::Falsy);
}

void ComparisonExpression::setFalsyLabel(symbols::AnnotationStore& store, symbols::LabelEntry falsyLabel) {
    store.setLabel(this, symbols::LabelSlot::Falsy, std::move(falsyLabel));
}

symbols::LabelEntry* ComparisonExpression::getTruthyLabel(symbols::AnnotationStore& store) const {
    return store.label(this, symbols::LabelSlot::Truthy);
}

void ComparisonExpression::setTruthyLabel(symbols::AnnotationStore& store, symbols::LabelEntry truthyLabel) {
    store.setLabel(this, symbols::LabelSlot::Truthy, std::move(truthyLabel));
}

} // namespace ast

