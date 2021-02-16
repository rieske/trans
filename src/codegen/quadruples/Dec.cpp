#include "Dec.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

Dec::Dec(std::string operandName) :
        operandName { operandName }
{
}

void Dec::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::string Dec::getOperandName() const {
    return operandName;
}

void Dec::print(std::ostream& stream) const {
    stream << "\tDEC " << getOperandName() << "\n";
}

} // namespace codegen
