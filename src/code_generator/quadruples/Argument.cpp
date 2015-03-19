#include "Argument.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

Argument::Argument(Value argument) :
        argument { argument }
{
}

void Argument::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

Value Argument::getArgument() const {
    return argument;
}

} /* namespace code_generator */
