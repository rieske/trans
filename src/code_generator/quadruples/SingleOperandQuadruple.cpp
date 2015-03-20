#include "SingleOperandQuadruple.h"

namespace code_generator {

SingleOperandQuadruple::SingleOperandQuadruple(std::string operand, std::string result) :
        operand { operand },
        result { result }
{
}

std::string SingleOperandQuadruple::getOperand() const {
    return operand;
}

std::string SingleOperandQuadruple::getResult() const {
    return result;
}

} /* namespace code_generator */
