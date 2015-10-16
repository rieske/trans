#include "codegen/quadruples/DoubleOperandQuadruple.h"

namespace codegen {

DoubleOperandQuadruple::DoubleOperandQuadruple(std::string leftOperandName, std::string rightOperandName, std::string resultName) :
        leftOperandName { leftOperandName },
        rightOperandName { rightOperandName },
        resultName { resultName }
{
}

std::string DoubleOperandQuadruple::getLeftOperandName() const {
    return leftOperandName;
}

std::string DoubleOperandQuadruple::getRightOperandName() const {
    return rightOperandName;
}

std::string DoubleOperandQuadruple::getResultName() const {
    return resultName;
}

} /* namespace code_generator */
