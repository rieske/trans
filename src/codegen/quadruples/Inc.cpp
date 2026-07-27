#include "Inc.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

Inc::Inc(std::string operandName, int step) :
        operandName { operandName },
        step { step }
{
}

void Inc::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::string Inc::getOperandName() const {
    return operandName;
}

void Inc::print(std::ostream& stream) const {
    if (step == 1) {
        stream << "\tINC " << getOperandName() << "\n";
    } else {
        stream << "\t" << getOperandName() << " := " << getOperandName() << " + " << step << "\n";
    }
}

} // namespace codegen
