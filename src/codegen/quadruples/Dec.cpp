#include "Dec.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

Dec::Dec(std::string operandName, int step) :
        operandName { operandName },
        step { step }
{
}

void Dec::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::string Dec::getOperandName() const {
    return operandName;
}

void Dec::print(std::ostream& stream) const {
    if (step == 1) {
        stream << "\tDEC " << getOperandName() << "\n";
    } else {
        stream << "\t" << getOperandName() << " := " << getOperandName() << " - " << step << "\n";
    }
}

} // namespace codegen
