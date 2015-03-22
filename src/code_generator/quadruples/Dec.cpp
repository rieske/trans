#include "Dec.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

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

} /* namespace code_generator */
