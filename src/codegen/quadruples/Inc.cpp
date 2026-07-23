#include "Inc.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

Inc::Inc(std::string operandName, int amount) :
        operandName { operandName },
        amount { amount }
{
}

void Inc::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::string Inc::getOperandName() const {
    return operandName;
}

void Inc::print(std::ostream& stream) const {
    if (amount == 1) {
        stream << "\tINC " << getOperandName() << "\n";
    } else {
        stream << "\tINC " << getOperandName() << " +" << amount << "\n";
    }
}


void Inc::collectSymbolRefs(SymbolRefs& refs) const {
    refs.addUse(operandName);
    refs.addDef(operandName);
}

} // namespace codegen

