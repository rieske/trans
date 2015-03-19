#include "SingleOperandQuadruple.h"

namespace code_generator {

SingleOperandQuadruple::SingleOperandQuadruple(Value operand, Value result) :
        operand { operand },
        result { result }
{
}

Value SingleOperandQuadruple::getOperand() const {
    return operand;
}

Value SingleOperandQuadruple::getResult() const {
    return result;
}

} /* namespace code_generator */
