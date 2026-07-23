#include "SingleOperandQuadruple.h"

namespace codegen {

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

void SingleOperandQuadruple::collectSymbolRefs(SymbolRefs& refs) const {
    refs.addUse(operand);
    refs.addDef(result);
}

} // namespace codegen
