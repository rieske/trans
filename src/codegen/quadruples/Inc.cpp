#include "Inc.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

Inc::Inc(std::string operandName) :
        operandName { operandName }
{
}

void Inc::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::string Inc::getOperandName() const {
    return operandName;
}

void Inc::print(std::ostream& stream) const {
    stream << "\tINC " << getOperandName() << "\n";
}

} // namespace codegen

