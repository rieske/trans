#include "SingleOperandExpression.h"

#include <algorithm>
#include "Operator.h"

namespace ast {

SingleOperandExpression::SingleOperandExpression(std::unique_ptr<Expression> _operand, std::unique_ptr<Operator> _operator) :
        _operand { std::move(_operand) }, _operator { std::move(_operator) } {
}

SingleOperandExpression::~SingleOperandExpression() {
}

const TranslationUnitContext& SingleOperandExpression::getContext() const {
    return _operand->getContext();
}

Expression* SingleOperandExpression::getOperand() const {
    return _operand.get();
}

Operator* SingleOperandExpression::getOperator() const {
    return _operator.get();
}

} /* namespace ast */
