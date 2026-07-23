#include "Dec.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

Dec::Dec(std::string operandName, int amount) :
        operandName { operandName },
        amount { amount }
{
}

void Dec::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::string Dec::getOperandName() const {
    return operandName;
}

void Dec::print(std::ostream& stream) const {
    if (amount == 1) {
        stream << "\tDEC " << getOperandName() << "\n";
    } else {
        stream << "\tDEC " << getOperandName() << " -" << amount << "\n";
    }
}


void Dec::collectSymbolRefs(SymbolRefs& refs) const {
    refs.addUse(operandName);
    refs.addDef(operandName);
}

} // namespace codegen
