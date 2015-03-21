#include "Argument.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

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

} /* namespace code_generator */
