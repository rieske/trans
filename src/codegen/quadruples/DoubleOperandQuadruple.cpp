#include "DoubleOperandQuadruple.h"

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

void DoubleOperandQuadruple::collectSymbolRefs(SymbolRefs& refs) const {
    refs.addUse(leftOperandName);
    refs.addUse(rightOperandName);
    refs.addDef(resultName);
}

} // namespace codegen
