#include "Argument.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

Argument::Argument(std::string argumentName) :
        argumentName { argumentName }
{
}

void Argument::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::string Argument::getArgumentName() const {
    return argumentName;
}

void Argument::print(std::ostream& stream) const {
    stream << "\tPARAM " << getArgumentName() << "\n";
}

} // namespace codegen

