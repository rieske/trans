#include "SingleOperandExpression.h"

#include "ParseEnvironment.h"

namespace ast {

SingleOperandExpression::SingleOperandExpression(std::unique_ptr<Expression> _operand) :
        _operand { std::move(_operand) }
{
}

SingleOperandExpression::~SingleOperandExpression() {
}

std::optional<type::Type> SingleOperandExpression::typeAtParseTime(const ParseEnvironment& environment) const {
    return _operand->typeAtParseTime(environment);
}

void SingleOperandExpression::visitOperand(AbstractSyntaxTreeVisitor& visitor) {
    _operand->accept(visitor);
}

type::Type SingleOperandExpression::operandType() const {
    return _operand->getType();
}

bool SingleOperandExpression::hasOperandSymbol(const symbols::AnnotationStore& store) const {
    return _operand->hasResultSymbol(store);
}

symbols::ValueEntry* SingleOperandExpression::operandSymbol(symbols::AnnotationStore& store) const {
    return _operand->getResultSymbol(store);
}

symbols::ValueEntry* SingleOperandExpression::operandLvalueSymbol(symbols::AnnotationStore& store) const {
    return _operand->getLvalueSymbol(store);
}

Expression* SingleOperandExpression::getOperandExpression() const {
    return _operand.get();
}

translation_unit::Context SingleOperandExpression::getContext() const {
    return _operand->getContext();
}

bool SingleOperandExpression::isLval() const {
    return _operand->isLval();
}

} // namespace ast

