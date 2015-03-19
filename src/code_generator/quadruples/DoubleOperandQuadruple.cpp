#include "DoubleOperandQuadruple.h"

namespace code_generator {

DoubleOperandQuadruple::DoubleOperandQuadruple(Value leftOperand, Value rightOperand, Value result) :
        leftOperand { leftOperand },
        rightOperand { rightOperand },
        result { result }
{
}

Value DoubleOperandQuadruple::getLeftOperand() const {
    return leftOperand;
}

Value DoubleOperandQuadruple::getRightOperand() const {
    return rightOperand;
}

Value DoubleOperandQuadruple::getResult() const {
    return result;
}

} /* namespace code_generator */
